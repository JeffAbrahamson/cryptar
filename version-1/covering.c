/*  covering.c
 *  Copyright (C) 2002, 2003, 2004 by:
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "checksum.h"
#include "covering.h"
#include "db_misc.h"
#include "prefs.h"
#include "summary.h"



static void cover_hash(FILE *fp, GPtrArray *old_blocks);
static void cover_complete_cover(FILE *fp, Signature *sig);
static gint block_compare_by_offset(gconstpointer v1, gconstpointer v2);



GPtrArray *cover_new()
{
        return g_ptr_array_new();
}



/* Determine what part of the old block list is usable for a new
   covering of the file.  Construct instructions for how to cover the
   file using as much as possible blocks already stored in the remote
   block store, using the file itself where no existing blocks are
   available.

   Those DBBlocks that are in the new covering *and* are modified must
   be stored in the local database.  The procedure is unfortunately
   somewhat complicated, since we can't save until we have a remote_id.

   If we already have a remote_id, we can save it now, because now is
   when we know that it has changed.  (In fact, the caller is
   responsible for calling cover_save_local.)  Otherwise, we save it
   when we get acknowledgement (and a remote_id) from the block server
   (put_ack).

   Input: existing file; block list currently in database

   Output: none

   Side-effect: Update sig.blockList
 */
void cover_update_remote(FILE *fp, Signature *sig)
{
        g_assert(fp);
        g_assert(sig->blockList);

        /* If old_remote is empty (no blocks to check), then we
           needn't bother at all with the hash.
         */
        if(sig->blockList->len) {
                WCsum_make_hash(fp, pref_get_int_value_silent_default(PREF_BLOCK_LENGTH,
                                                                      DEFAULT_BLOCK_LENGTH));
                cover_hash(fp, sig->blockList);
                hash_destroy(); /* won't be needing hash anymore */
        }

        cover_complete_cover(fp, sig);
        return;
}




/* Given an old covering, compare to the existing file and produce
   what partial covering we might from the old covering.  The new
   covering is simply indicated in the old one by checking the
   in_new_covering field.  Note that we use a hash map of weak
   checksums of all blocks of the existing file to avoid having to
   compute strong checksums too often.
*/
static void cover_hash(FILE *fp, GPtrArray *blocks)
{
        guint32 i, offset;
        DBBlock *block;
        
        assert(fp);
        assert(blocks);

        for(i = 0; i < blocks->len; i++) {
                block = g_ptr_array_index(blocks, i);
                if(hash_find(block->weak_checksum, &offset)
                   && SCsum_match(fp, offset, block)) {
                        block->in_new_cover = 1;
                        if(block->offset != offset) {
                                block->offset = offset;
                                block->local_id = 0; /* have to resave if position changed */
                        }
                }
        } /* for */
        return;
}



/* Fill in the holes of a partial covering using blocks from the local
   file.  As we add new blocks to the cover, we only indicate where
   and how much of the file to put in the blocks.  The data buffer is
   filled as it is needed.  This presents increased risk of data skew
   (the file could change while we're working), but is a useful memory
   usage optimization.
 */
static void cover_complete_cover(FILE *fp, Signature *sig)
{
        static guint32 block_length = 0;
        
        guint32 len, so_far;
        guint32 i;
        DBSummary *dbs;
        GPtrArray *covering;
        DBBlock *dbb, *block;
        
        assert(fp);
        assert(sig);
        assert(dbs = sig->summary);
        assert(covering = sig->blockList);

        if(!block_length)
                block_length = pref_get_int_value_silent_default(PREF_BLOCK_LENGTH,
                                                                 DEFAULT_BLOCK_LENGTH);

        /* Sort the part of the array that we need for completing the
           covering.  We'll append blocks to the end, which thus won't
           be in sorted order, but we won't scan past the end of the
           sorted portion.  Thus, the algorithm works even though the
           array won't stay in sorted.
        */
        g_ptr_array_sort(covering, block_compare_by_offset);
        len = covering->len;
        so_far = 0;
        for(i = 0; i < len; i++) {
                block = ((DBBlock *)g_ptr_array_index(covering, i));
                while(so_far < block->offset) {
                        dbb = db_block_new();
                        dbb->offset = so_far;
                        dbb->length = block_length;
                        dbb->in_new_cover = 1;
                        g_ptr_array_add(covering, dbb);
                        so_far += block_length;
                }
                if((block->offset <= so_far) && block->in_new_cover)
                        so_far = MAX(so_far, block->offset + block->length);
        }
       
        while(so_far < dbs->file_length) {
                dbb = db_block_new();
                dbb->offset = so_far;
                dbb->length = MIN(block_length, dbs->file_length - dbb->offset);
                if(dbb->length < block_length) {
                        /* End of file, so see if we can push back
                           beginning of block to make it full length. */
                        if(dbs->file_length > block_length)
                                dbb->offset -= block_length - dbb->length;
                }
                dbb->in_new_cover = 1;
                g_ptr_array_add(covering, dbb);
                so_far += dbb->length;
        }
        return;
}



static gint block_compare_by_offset(gconstpointer v1, gconstpointer v2)
{
        const DBBlock *b1, *b2;

        g_assert(v1);
        g_assert(v2);
        b1 = *(DBBlock **)v1;
        b2 = *(DBBlock **)v2;

        return (b1->offset == b2->offset) ? 0 :
                ((b1->offset < b2->offset) ? -1 : 1);
}



/* Fill in the details of a DBBlock.  The point of this is that we
   queue the blocks without filling in the data (and so also not the
   checksums) to save space in the work list.  When the work list is
   ready for this block, we fill it in.
 */
void cover_fill_block(DBBlock *dbb, FILE *fp)
{
        size_t num;
        SCsum sha;
        
        g_assert(dbb);
        g_assert(fp);
        g_assert(dbb->length);
        g_assert(dbb->local_id);

        dbb->data = g_malloc(dbb->length);
        fseek(fp, dbb->offset, SEEK_SET);
        if(ferror(fp)) {
                fprintf(stderr, "Failed to seek to %d in stream.\n", dbb->offset);
                return;         /* ### How do we flag error?  How do we recover? */
        }
        clearerr(fp);
        num = fread(dbb->data, 1, dbb->length, fp);
        if(num != dbb->length && ferror(fp)) {
                perror("Error reading file (short read): ");
                fprintf(stderr, "Attempting to continue...\n");
                clearerr(fp);
        }
        dbb->weak_checksum = WCsum_get(dbb->data, num);
        memmove(dbb->strong_checksum, SCsum_buffer(dbb->data, num, sha), SCsumLength);

        return;
}



/* Save in the local database those blocks modified for the new cover
   that we can save.  That is, those blocks that don't have a local_id
   key but that are already in the remote block store (have a
   remote_id key).
 */
void cover_save_local(GPtrArray *block_list)
{
        guint32 i;
        DBBlock *dbb;
        
        g_assert(block_list);
        
        for(i = 0; i < block_list->len; i++) {
                dbb = g_ptr_array_index(block_list, i);
                if(!dbb->local_id) {
                        dbb->local_id = db_get_next_index(kBlocks);
                        if(dbb->remote_id)
                                db_block_add(dbb);
                }
                        
        }
}


