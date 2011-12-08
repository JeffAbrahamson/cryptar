/*  ios.c
 *  Copyright (C) 2002, 2003 by:
 *  Jeff Abrahamson
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
*/


#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

#include "ios.h"


#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif




/* Buffers that grow as needed, with interfaces for writing stuff as
   we would want to over a network connection.

   In general, return 0 on success, non-zero on failure.
*/


struct ios_buf {
        char *buf;
        guint32 len;                /* how much data */
        guint32 read_ptr;
        guint32 alloc_len;          /* how much space reserved */
};


#define IOS_MIN_SIZE 1024
#define GROWTH_FACTOR 1.2
#ifdef MAINTAINER_MODE
GPtrArray *ios_tracker = NULL;  /* debug memory allocation */
static void ios_free_debug(IOSBuf *ios);
#endif

IOSBuf *ios_new()
{
        IOSBuf *ios;

        ios = g_malloc(sizeof(IOSBuf));
        ios->len = 0;
        ios->read_ptr = 0;
        ios->alloc_len = IOS_MIN_SIZE;
        ios->buf = g_malloc(ios->alloc_len);
#ifdef MAINTAINER_MODE
        if(!ios_tracker)
                ios_tracker = g_ptr_array_new();
        g_ptr_array_add(ios_tracker, ios);
#endif
        return(ios);
}



IOSBuf *ios_new_copy(IOSBuf *ios_src)
{
        IOSBuf *ios_out;
        
        g_assert(ios_src);
        ios_out = ios_new();
        ios_append_ios(ios_out, ios_src);
        return ios_out;
}



void ios_free(IOSBuf *ios)
{
        g_assert(ios);
        g_free(ios->buf);
        g_free(ios);
#ifdef MAINTAINER_MODE
        ios_free_debug(ios);
#endif
        return;
}




#ifdef MAINTAINER_MODE
static void ios_free_debug(IOSBuf *ios)
{
        unsigned int i;
        
        g_assert(ios_tracker);
        for(i = 0; i < ios_tracker->len; i++) {
                if(ios_tracker->pdata[i] == ios) {
                        g_ptr_array_index(ios_tracker, i) = NULL;
                        return;
                }
	}
        g_error("IOSBuf free: pointer not found when checking allocation array");
        return;                 /* not executed */
}
#endif



void ios_grow(IOSBuf *ios, guint32 size)
{
        assert(ios);
        assert(ios->buf);

        if(ios->read_ptr) {
                memmove(ios->buf, &ios->buf[ios->read_ptr],
                        ios->len - ios->read_ptr);
                ios->len -= ios->read_ptr;
                ios->read_ptr = 0;
        }
        if(ios->alloc_len < size) {
                ios->alloc_len = size;
                ios->buf = g_realloc(ios->buf, ios->alloc_len);
        }
        return;
}




void ios_append(IOSBuf *ios, const void *data, int size)
{
        g_assert(ios);
        g_assert(size >= 0);
        if(!size)
                return;
        g_assert(data);
        if(ios->len + ios->read_ptr + size > ios->alloc_len)
                ios_grow(ios, (ios->len + size) * GROWTH_FACTOR);
        memmove(&ios->buf[ios->len], data, size);
        ios->len += size;
        return;
}



void ios_append_ios(IOSBuf *ios, IOSBuf *ios_src)
{
        g_assert(ios);
        g_assert(ios_src);
        ios_append(ios, ios_src->buf, ios_src->len);
        return;
}



void *ios_at(IOSBuf *ios, guint32 offset)
{
        g_assert(ios);
        if((offset > ios_buffer_size(ios)) || (ios_buffer_size(ios) == 0))
                return NULL;
        return &ios->buf[ios->read_ptr + offset];
}



void *ios_read(IOSBuf *ios, guint32 len)
{
        void *ptr;
        
        assert(ios);
        /* assert(len >= 0); */
        if(ios->read_ptr + len > ios->len)
                return NULL;
        ptr = &ios->buf[ios->read_ptr];
        ios->read_ptr += len;
        return ptr;
}



void ios_reset(IOSBuf *ios)
{
        assert(ios);
        ios->len = 0;
        ios->read_ptr = 0;
}



void ios_append_int8(IOSBuf *ios, gint8 num)
{
        assert(ios);
        ios_append(ios, (char *)&num, sizeof(num));
        return;
}



void ios_append_int16(IOSBuf *ios, gint16 num)
{
        assert(ios);
        num = htons(num);
        ios_append(ios, (char *)&num, sizeof(num));
        return;
}



void ios_append_int32(IOSBuf *ios, gint32 num)
{
        assert(ios);
        num = htonl(num);
        ios_append(ios, (char *)&num, sizeof(num));
        return;
}



void ios_append_string(struct ios_buf *ios, const char *str)
{
        guint32 len;

        assert(ios);
        assert(str);
        len = strlen(str);
        ios_append_int32(ios, len);
        ios_append(ios, str, len);
        return;
}



guint16 ios_int16_at(IOSBuf *ios, guint32 offset)
{
        char *bp;
        
        assert(ios);
        bp = ios_buffer(ios);
        return ntohs((gint16)*(gint16 *)&bp[offset]);
}



guint16 ios_read_int16(IOSBuf *ios)
{
        gint16 num;
        assert(ios);

        num = ntohs((gint16)*(gint16 *)&ios->buf[ios->read_ptr]);
        ios->read_ptr += sizeof(gint16);
        assert(ios->read_ptr <= ios->len);
        return num;
}



guint32 ios_int32_at(IOSBuf *ios, guint32 offset)
{
        char *bp;
        
        assert(ios);
        bp = ios_buffer(ios);
        return ntohl((gint32)*(gint32 *)&bp[offset]);
}



guint32 ios_read_int32(IOSBuf *ios)
{
        gint32 num;
        assert(ios);

        num = ntohl((gint32)*(gint32 *)&ios->buf[ios->read_ptr]);
        ios->read_ptr += sizeof(gint32);
        assert(ios->read_ptr <= ios->len);
        return num;
}



char *ios_read_str(struct ios_buf *ios)
{
        guint32 len;
        
        assert(ios);
        len = ios_read_int32(ios);
        if(len) {
                return g_strndup(ios_read(ios, len), len);
        }
        return NULL;            /* don't allocate if nothing there */
}



void *ios_buffer(IOSBuf *ios)
{
        assert(ios);

        return ios_at(ios, 0);
}



guint32 ios_buffer_size(IOSBuf *ios)
{
        assert(ios);

        return ios->len - ios->read_ptr;
}



void ios_set_buffer(IOSBuf *ios, const void *data, guint32 size)
{
        assert(ios);
        assert(data);

        ios_reset(ios);
        ios_append(ios, data, size);
        return;
}



void ios_copy_ios(IOSBuf *ios_new, IOSBuf *ios_src)
{
        g_assert(ios_new);
        g_assert(ios_src);
        ios_reset(ios_new);
        ios_append_ios(ios_new, ios_src);
        return;
}
