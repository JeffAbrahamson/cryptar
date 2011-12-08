/*  block.c
 *  Copyright (C) 2002, 2003, 2004 by:
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
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "checksum.h"
#include "db_misc.h"
#include "ios.h"
#include "block.h"
#include "options.h"



void db_block_add(DBBlock *b)
{
        IOSBuf *key;
        IOSBuf *data;

        assert(b);
        
        key = ios_new();
        ios_append_int32(key, b->local_id);
        
        data = ios_new();
        ios_append(data, &(b->version), sizeof(b->version));
        ios_append_int32(data, b->offset);
        ios_append_int16(data, b->length);

        db_put(kBlocks, key, data);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("saved block: local=%d remote=%d offset=%d wc=%x",
                          b->local_id, b->remote_id, b->offset, b->weak_checksum);
        ios_free(key);
        ios_free(data);
        return;
}



DBBlock *db_block_fetch(guint32 id)
{
        IOSBuf *key;
        IOSBuf *data;

        key = ios_new();
        ios_append_int32(key, id);
        data = ios_new();

        db_get(kBlocks, key, data);

        ios_free(key);
        ios_free(data);
        
        return db_block_from_stream(data, id);
}



void db_block_to_stream(DBBlock *dbb, IOSBuf *ios)
{
        g_assert(dbb);
        g_assert(ios);
        db_block_to_stream_no_data(dbb, ios);
        g_assert(dbb->data);
        ios_append(ios, dbb->data, dbb->length);
        return;
}



void db_block_to_stream_no_data(DBBlock *dbb, IOSBuf *ios)
{
        g_assert(dbb);
        g_assert(ios);
        ios_append_int8(ios, dbb->version);
        ios_append_int32(ios, dbb->remote_id);
        ios_append_int32(ios, dbb->offset);
        ios_append_int16(ios, dbb->length);
        ios_append_int32(ios, dbb->weak_checksum);
        ios_append(ios, dbb->strong_checksum, 20);
        return;
}



DBBlock *db_block_from_stream(IOSBuf *ios, const guint32 id)
{
        DBBlock *dbb;

        assert(ios);
        if(!ios_buffer(ios))
                return NULL;
        /* ### Verify that ios is long enough? ### */
        dbb = db_block_from_stream_no_data(ios, id);
        dbb->data = g_memdup(ios_read(ios, dbb->length), dbb->length);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("block from stream: local=%d remote=%d offset=%d wc=%x",
                          dbb->local_id, dbb->remote_id,
                          dbb->offset, dbb->weak_checksum);

        return dbb;
}



DBBlock *db_block_from_stream_no_data(IOSBuf *ios, const guint32 id)
{
        DBBlock *dbb;

        assert(ios);
        /* ### Verify that ios is long enough? ### */
        dbb = db_block_new();
        dbb->local_id = id;
        dbb->version = ((char *)(ios_read(ios, 1)))[0];
        dbb->remote_id = ios_read_int32(ios);
        dbb->offset = ios_read_int32(ios);
        dbb->length = ios_read_int16(ios);
        dbb->weak_checksum = ios_read_int32(ios);
        memcpy(dbb->strong_checksum, ios_read(ios, 20), 20);
        return dbb;
}



DBBlock *db_block_new()
{
        DBBlock *dbb;

        dbb = g_new0(DBBlock, 1);
        return dbb;
}



void db_block_free(DBBlock *dbb)
{
        assert(dbb);
        if(dbb->data)
                g_free(dbb->data);
        g_free(dbb);
        return;
}



void db_block_display(DBBlock *dbb)
{
        int i;
        
        assert(dbb);
        printf("DBBlock:\n");
        printf("  dbb->local_id = %d\n", dbb->local_id);
        printf("  dbb->version = %d\n", dbb->version);
        printf("  dbb->remote_id = %d\n", dbb->remote_id);
        printf("  dbb->offset = %d\n", dbb->offset);
        printf("  dbb->length = %d\n", dbb->length);
        printf("  dbb->weak_checksum = %d\n", dbb->weak_checksum);
        printf("  dbb->strong_checksum = ");
        for(i = 0; i < 20; i += 2)
                printf("%2.2X%2.2X ",
                       dbb->strong_checksum[i],
                       dbb->strong_checksum[i+1]);
        printf("\n");
        printf("  dbb->data = <omitted>\n");
        return;
}


