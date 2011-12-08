/*  workticket.h
 *  Copyright (C) 2002-2004 by:
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


#ifndef __WORKTICKET_H__
#define __WORKTICKET_H__

#include "summary.h"


struct db_file;
struct db_block;
struct signature;
struct ios_buf;


enum wt_actions {
        wt_Archive,
        wt_Extract
};

enum wt_status {
        wt_New,
        wt_AwaitingBlockList,
        wt_SendingBlocks,       /* Archive-only */
        wt_SendingBlockRequests, /* Extract-only */
        wt_AwaitingBlockConfirms,
        wt_AwaitingBlockListConfirm
};


typedef struct work_ticket {
        struct db_file *dbf;
        struct signature sig;
        guint32 num_blocks_moved; /* Set initially to zero,
                                     incremented each time we send a
                                     block (archive mode) or send a
                                     block request (extract
                                     mode). Each time we receive an
                                     ack, we decrement it. We are done
                                     when equal to zero *and* status
                                     is wt_AwaitingBlockConfirms or
                                     wt_AwaitingBlockListConfirm. */
        guint32 next_block_to_queue;
        enum wt_status status;
        FILE *fp;
        enum wt_actions action;
} WorkTicket;


WorkTicket *new_work_ticket();
void wt_insert(WorkTicket *wt);
void wt_remove(WorkTicket *wt);
int wt_more();
int wt_more_to_come();
void wt_working(int x);
WorkTicket *wt_find(guint32 file_id);
struct db_block *wt_find_block(WorkTicket *wt, guint32 block_id);
void wt_finish(WorkTicket *wt);
void wt_process_queue(struct ios_buf *ios);
int wt_queue_full();
void wt_check_for_orphan_work();
void wt_assert(WorkTicket *wt);

#endif
