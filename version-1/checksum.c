/* checksum.c
   Copyright (C) Jeff Abrahamson 2002, 2003, 2004
   Copyright (C) Andrew Tridgell 1996
   Copyright (C) Paul Mackerras 1996
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <assert.h>
#include <errno.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "block.h"
#include "byteorder.h"
#include "checksum.h"
#include "hash.h"


#define DEBUG_WCSUM 1

/* Notation: WCsum is the weak checksum, SCsum is the strong checksum (sha1) */

static void WCsum_make_hash_sub(FILE *fp, guint16 block_len, guint16 init_offset);
static int WCsum_buffer_update(FILE *fp, char *b, guint16 len);
#if DEBUG_WCSUM
static void WCsum_verify(const char *last_buf, const char *buf, int i, guint32 buf_len, guint32 block_len, guint32 csum);
#endif


#define CHAR4_TO_INT32(b) ((b[0] << 24) + (b[1] << 16) + (b[2] << 8) + (b[3]))
#define M16 0xffff

/* Rolling checksum from Tridgell's PhD thesis. */
guint32 WCsum_get(signed char *buf, guint16 len)
{
        guint32 s, i;
        guint32 s1, s2;
        guint32 len4;
        char *p;
        
        // assert(len > 4);
        s1 = s2 = 0;
        len4 = len >> 2;
        for(i = 0; i < len; i += 4) {
                p = &buf[i];
                s = CHAR4_TO_INT32(p);
                s1 += s;
                /* s2 += (len4 - 4 * i) * s; */
                s2 += (len4 - (i >> 2)) * s;
        }
        s2 = (s2 & M16);
        return (s2 << 16) + (s1 & M16);
}



guint32 WCsum_update(signed char *old4, signed char *new4, guint32 old_sum, guint32 len)
{
        guint32 s_old, s_new;
        guint32 s1, s2;

        s_old = CHAR4_TO_INT32(old4);
        s_new = CHAR4_TO_INT32(new4);
        s1 = (old_sum & M16) - s_old + s_new;
        /* s2 = ((old_sum >> 16) & M16) - (len >> 2) * s_old + s1 * s_new; */
        s2 = ((old_sum >> 16) & M16);
        s2 = ((s2 - (len >> 2) * s_old) & M16) + s1;
        s2 = (s2 & M16);
        return (s2 << 16) + (s1 & M16);
}



#define MIN_BUF_LEN 8192

/* Make a hash table (we only have one, so we don't need to pass its
   address around) of weak checksums of the file fp. We will have an
   entry for every offset from 0 to file_length - block_len.

   Caller should make sure file is at least block_len bytes long,
   since we don't need a hash table if the file is so short.
*/
void WCsum_make_hash(FILE *fp, guint16 block_len)
{
        /* 32 bit checksum means we have to do four times. If this is
           a bottleneck, rewrite to be more efficient. But the update
           procedure (rolling window method) is by 32 bit word, not by
           byte. */
        hash_create();
        WCsum_make_hash_sub(fp, block_len, 0);
        WCsum_make_hash_sub(fp, block_len, 1);
        WCsum_make_hash_sub(fp, block_len, 2);
        WCsum_make_hash_sub(fp, block_len, 3);
        return;
}



/* Result will be extra work, since we'll have more hash misses. But
   at least we will still be able to continue.
*/
static void WCsum_make_hash_sub(FILE *fp, guint16 block_len, guint16 init_offset)
{
        static char *buf = 0;
        static char *last_buf = 0;
        static guint16 buf_len = 0;

        char *tmp_buf_ptr;
        guint32 csum, offset, i;
        size_t num;

        g_assert(block_len);
        g_assert(0 == block_len % 4);
        g_assert(block_len >= 4);
        /* algorithm fails if buf_len is less than block_len */
        if(buf_len != block_len) {
                buf_len = block_len;
                buf = g_realloc(buf, buf_len);
                last_buf = g_realloc(last_buf, block_len);
        }

        if(fseek(fp, init_offset, SEEK_SET)) {
                g_warning("fseek returned error in WCsum_make_hash_sub: %s",
                          strerror(errno));
                return;
        }
        offset = init_offset;
        num = fread(buf, 1, block_len, fp);
        if(num < block_len) {
                if(!feof(fp))
                        g_warning("Read error in WCsum_make_hash_sub");
                return;
        }
        csum = WCsum_get(buf, block_len); /* initialize for loop */
        i = num;
        while(1) {
                hash_insert(csum, offset);
                if(i + 4 > num) {
                        if(num != block_len)
                                return; /* EOF or error */
                        tmp_buf_ptr = last_buf;
                        last_buf = buf;
                        buf = tmp_buf_ptr;
                        num = WCsum_buffer_update(fp, buf, block_len);
                        if(num < 4) /* only care about multiples of 4 */
                                return;
                        if((num != block_len) && !feof(fp))
                                g_warning("Short read not at end of file: "
                                          "WCsum_make_hash");
                        i = 0;
                }
                if(i + 4 <= num) {
                        csum = WCsum_update(&last_buf[i], &buf[i],
                                                    csum, block_len);
                        i += 4;
#if DEBUG_WCSUM
                        WCsum_verify(last_buf, buf, i, buf_len, block_len, csum);
#endif
                }
                offset += 4;
        }
        return;
}



static int WCsum_buffer_update(FILE *fp, char *b, guint16 len)
{
        int num;

        g_assert(fp);
        g_assert(b);
        num = fread(b, 1, len, fp);
        if(num < len && !feof(fp)) {
                g_warning("Read error in WCsum_buffer_update");
                return 0;
        }
        if(num < len)
                memset(&b[num], 0, len - num);
        return num;
}



#if DEBUG_WCSUM
static void WCsum_verify(const char *last_buf, const char *buf, int i, guint32 buf_len, guint32 block_len, guint32 csum)
{
        /* Verify that update of weak checksum produces the data we
           think it does. */
        char *buf_verify;
        int first, second;
        guint32 csum_verify;
        
        buf_verify = g_malloc(block_len);
        first = MIN(block_len, buf_len - i);
        second = block_len - first;
        memcpy(buf_verify, &last_buf[i], first);
        memcpy(&buf_verify[first], buf, second);
        csum_verify = WCsum_get(buf_verify, block_len);
        g_assert(csum == csum_verify);
        g_free(buf_verify);
}
#endif



SCsumPtr SCsum_buffer(const unsigned char *buf, guint16 len, SCsumPtr sc)
{
        char *ret;
        ret = SHA1(buf, len, sc);
        g_assert(ret);
        g_assert(memcmp(ret, sc, SCsumLength) == 0);
        return ret;
}



#define SCsum_chunk_size 8196

void SCsum_file(FILE *fp, SCsumPtr sc)
{
        guint32 len;
        char buf[SCsum_chunk_size];
        SHA_CTX c;

        g_assert(fp);
        g_assert(sc);
        SHA1_Init(&c);
        rewind(fp);
        while(!feof(fp) && !ferror(fp)) {
                len = fread(buf, 1, SCsum_chunk_size, fp);
                SHA1_Update(&c, buf, len);
        }
        SHA1_Final(sc, &c);
        if(ferror(fp))
                g_warning("Error computing strong checksum for file. Continuing.");

        return;
}



/* Compare the strong checksum of the file at the offset with the
   stored strong checksum of the block. Return 0 if they don't match,
   non-zero if they do. */
int SCsum_match(FILE *fp, guint32 offset, DBBlock *block)
{
        size_t num;
        static char *buf = NULL;
        static guint16 buf_len;
        SCsum hash;
        
        assert(fp);
        assert(block);

        if(!buf || (buf_len < block->length)) {
                if(buf) g_message("Reallocating buf in SCsum_match.");
                buf_len = block->length;
                buf = g_realloc(buf, buf_len);
        }

        if(fseek(fp, offset, SEEK_SET)) {
                perror("SCsum_match: fseek");
                exit(1); /* #### temp #### */
        }
        num = fread(buf, 1, block->length, fp);
        if(num != block->length) {
                /* if not, it's not a match anyway, so don't worry. */
                return 0;
        }
        return SCsum_cmp(SCsum_buffer(buf, num, hash), block->strong_checksum);
}



/* Return true on match, false otherwise. */
int SCsum_cmp(SCsumPtr sc1, SCsumPtr sc2)
{
        g_assert(sc1);
        g_assert(sc2);
        return (memcmp(sc1, sc2, SCsumLength) == 0);
}



/* ### We assume SCsumLength % 4 == 0 here. ### */
void SCsum_print(SCsumPtr sc)
{
        int i = 0;

        while(i < SCsumLength) {
                printf("%2.2X%2.2X ", sc[i], sc[i+1]);
                i += 2;
                if(i == 10)
                        printf(" ");
        }
        return;
}

