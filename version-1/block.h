/*  block.h
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


#ifndef __LOCAL__
#define __LOCAL__

#include <glib.h>
#include <db.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "checksum.h"

struct ios_buf;


/* DBBlocks have several uses.

   We keep them in the local database to tell us how to fetch and
   authenticate blocks from the remote store.  They also tell us how
   to compare the blocks in the remote store (and so the virtual file
   that we are comparing against) with the blocks of a local file.

   We also use them as a sort of edit script, telling us how to move
   blocks around to transform one version of a file into another.  The
   in_new_cover field tells us in this usage whether the block is part
   of the new file or not.

   A fully-formed DBBlock has both its local_id and its remote_id
   filled in.  (The data pointer is retrieved only when needed and
   freed after that need is done.)

   If the local_id is zero, then the DBBlock is not yet saved locally.
   The remote_id may or may not be non-zero in this case: it could be
   that the block is stored remotely, but its offset has changed, so
   it needs to be restored locally.  Or it could be new and need to be
   stored remotely.

   If the remote_id is zero, then the data block has not yet been
   saved in the remote block store.
 */
typedef struct db_block {
        /* Local database portion */
        guint32 local_id;       /* primary key in local database, if
                                   zero then record is not yet stored
                                   locally. */
        guint8 version;
        guint32 offset;         /* where in file this block starts */
        guint16 length;         /* how long this block is.  It could
                                   be short if the file is shorter
                                   than our preferred block length.
                                   It could be long if it corresponds
                                   to a block list. */
        guint32 weak_checksum;
        SCsum strong_checksum;

        /* Remote database portion */
        guint32 remote_id;      /* primary key in remote block store
                                   (how to request it).  If zero then
                                   block is not yet stored remotely.
                                   Stored both locally and
                                   remotely. */
        void *data;             /* Only stored remotely. */

        /* Non-persistent administrative portion */
        gboolean block_queued;
        gboolean block_acked;
        gboolean in_new_cover;
} DBBlock;


void db_block_add(DBBlock *b);
DBBlock *db_block_fetch(guint32 id);
void db_block_to_stream(DBBlock *dbb, struct ios_buf *ios);
void db_block_to_stream_no_data(DBBlock *dbb, struct ios_buf *ios);
DBBlock *db_block_from_stream(struct ios_buf *ios, const guint32 id);
DBBlock *db_block_from_stream_no_data(struct ios_buf *ios, const guint32 id);
DBBlock *db_block_new();
void db_block_free(DBBlock *dbb);
void db_block_display(DBBlock *dbb);

#endif

