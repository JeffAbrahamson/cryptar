/*  requests.c
 *  Copyright (C) 2002, 2003 by:
 *  Jeff Abrahamson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "block.h"
#include "checksum.h"
#include "compress.h"
#include "encryption.h"
#include "filename.h"
#include "ios.h"
#include "options.h"
#include "protocol.h"
#include "requests.h"
#include "workticket.h"


static IOSBuf *block_list_to_stream(Signature *sig);



void request_block_list(WorkTicket *wt)
{
        ProtoGetBlock gb;
        
        assert(wt);
        memset(&gb, 0, sizeof(gb));
        gb.file_id = wt->dbf->file_id;
        gb.block_id = 0;
        gb.archive_id = wt->sig.summary->block_list_remote_id;
        send_get_block(&gb);
        return;
}



void send_block_list(Signature *sig)
{
        IOSBuf *ios, *ios1, *ios2;
        ProtoPutBlock pb;
        guint32 length;
        void *data;
        
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("sending block list");
        g_assert(sig);
        ios = block_list_to_stream(sig);
        g_assert(ios);
        SCsum_buffer(ios_buffer(ios), ios_buffer_size(ios),
                     sig->summary->block_list_sha1);
        memset(&pb, 0, sizeof(pb));
        pb.file_id = sig->summary->file_id;
        pb.block_id = 0;
        length = ios_buffer_size(ios);
        data = ios_buffer(ios);
        sig->summary->block_list_length = length;
        ios1 = ios_new();
        ios_set_buffer(ios1, data, length);
        ios2 = do_compress(ios1);
        pb.ios = encrypt(ios2);
        ios_free(ios1);
        ios_free(ios2);
        send_put_block(&pb);
        ios_free(ios);
        return;
}



static IOSBuf *block_list_to_stream(Signature *sig)
{
        IOSBuf *ios;
        GPtrArray *block_list;
        DBBlock *dbb;
        guint i;

        g_assert(sig);
        block_list = sig->blockList;
        g_assert(block_list);   /* can't fail */
        ios = ios_new();
        ios_append_int32(ios, block_list->len);
        for(i = 0; i < block_list->len; i++) {
                dbb = g_ptr_array_index(block_list, i);
                g_assert(dbb);
                ios_append_int32(ios, dbb->local_id);
                db_block_to_stream_no_data(dbb, ios);
        }
        return ios;
}



GPtrArray *block_list_from_stream(IOSBuf *ios)
{
        GPtrArray *block_list;
        guint32 len, i, id;
        DBBlock *dbb;
        
        g_assert(ios);
        block_list = g_ptr_array_new();
        len = ios_read_int32(ios);
        for(i = 0; i < len; i++) {
                id = ios_read_int32(ios);
                dbb = db_block_from_stream_no_data(ios, id);
                g_ptr_array_add(block_list, dbb);
        }
        return block_list;
}
