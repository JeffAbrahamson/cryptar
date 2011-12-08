/*  workticket.c
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


#include <glib.h>
#include <string.h>

#include "block.h"
#include "compress.h"
#include "covering.h"
#include "db_misc.h"
#include "encryption.h"
#include "filename.h"
#include "io.h"
#include "ios.h"
#include "options.h"
#include "protocol.h"
#include "queue.h"
#include "summary.h"
#include "workticket.h"


/* The work list contains worktickets.  The point of this is that we
   can't realistically all the data from each file in memory at once,
   and yet we can surely compute data for network activity faster than
   the network can handle it.  So we queue notes (worktickets) about
   what needs to be done.  When the network output routine wants more
   data in its queue, it will ask us to do some work until it's got
   enough data to make it happy.
 */



/* The principal limit on the queue size is how many open files we're
   willing to have hanging about.  Each item in the queue has a FILE
   pointer so that it can fill blocks with data when asked by the
   output queue. */
#define WT_MAX_QUEUE_SIZE 20;

/* This is the work list.  For historical reasons, it's unfortunately
   also called the work queue.  Some day I should actually search for
   all the instances of "work queue" and change them to "work list."

   When NULL, there's no work queued up.
 */
static GSList *work_list = 0;

/* The work list may contain notes of work done awaiting confirmation,
   but no actual work to do.  So more_work is true if there might be
   more work to do, false if we know there's none (and so don't need
   to scan the list).
 */
static char more_work = 0;


static gint find_wt_by_file_id(gconstpointer a, gconstpointer b);
static void wt_do_work(gpointer data, gpointer user_data);
static void wt_send_blocks(WorkTicket *wt, IOSBuf *ios);
static void wt_send_block_requests(WorkTicket *wt, IOSBuf *ios);
#if DUMB
static int wt_check_if_done();
static void wt_count_remaining(gpointer data, gpointer user_data);
#endif



/* Make a blank workticket.
 */
WorkTicket *new_work_ticket()
{
        WorkTicket *wt;

        wt = g_new0(WorkTicket, 1);
        wt->status = wt_New;
        return wt;
}



/* Insert a new workticket into the work list for consideration by the
   I/O loop.
 */
void wt_insert(WorkTicket *wt)
{
        g_assert(wt);

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Inserting work ticket 0x%lX", (unsigned long)wt);
        if(wt_queue_full())
                g_warning("Inserting work ticket even though the work list is full.");
        work_list = g_slist_append(work_list, wt);
        /* We need to reference the write buffer just to make sure
           it's registered, otherwise the main event loop might not
           consider it for output. */
        (void)io_write_buf();
        return;
}



/* Remove a workticket from the work list.
 */
void wt_remove(WorkTicket *wt)
{
        g_assert(wt);

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Removing work ticket 0x%lX", (unsigned long)wt);
        work_list = g_slist_remove(work_list, wt);
        return;
}



/* Return true if at least one workticket is in the work list.
 */
int wt_more()
{
        return (work_list != 0);
}



/* This feels like a kludge. We want to be able to distinguish between
   an empty work list because work processing has gotten ahead of work
   generation and an empty work list because we're done. So archive
   and so forth call wt_working(1) to indicate that they're busy, and
   wt_working(0) to indicate that they're done. Then we can quit if
   !wt_more() && !wt_more_to_come(). So ugly...
*/
int wt_more_to_come()
{
        return more_work;
}



void wt_working(int x)
{
        more_work = x;
}



/* Retrieve a workticket from the work list by its ID.
 */
WorkTicket *wt_find(guint32 file_id)
{
        GSList *gsl;
        WorkTicket *wt;
        g_assert(file_id);

        gsl = g_slist_find_custom(work_list,
                                  GUINT_TO_POINTER(file_id),
                                  find_wt_by_file_id);
        g_assert(gsl);
        wt = gsl->data;
        g_assert(wt);
        return wt;
}



/* g_slist_find_custom callback: return 0 if found, non-zero otherwise. */
static gint find_wt_by_file_id(gconstpointer a, gconstpointer b)
{
        WorkTicket *wt;
        guint32 file_id;

        wt = (WorkTicket *)a;
        file_id = GPOINTER_TO_UINT(b);
        return (wt->dbf->file_id == file_id) ? 0 : 1;
}



/* Find a block (by id) in a given workticket.  Return the block.
 */
DBBlock *wt_find_block(WorkTicket *wt, guint32 block_id)
{
        GPtrArray *bl;
        guint32 i;
        DBBlock *dbb;
        
        g_assert(wt);
        bl = wt->sig.blockList;
        g_assert(bl);
        for(i = 0; i < bl->len; i++) {
                dbb = g_ptr_array_index(bl, i);
                g_assert(dbb);
                if(dbb->local_id == block_id)
                        return dbb;
        }
        return NULL;
}



/* Work ticket is finished, so write summary to database and remove
   from the work list.
*/
void wt_finish(WorkTicket *wt)
{
        DBSummary *dba;

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Finishing work ticket 0x%lX, file id %d", (unsigned long)wt, wt->dbf->file_id);
        g_assert(wt);
        dba = wt->sig.summary;
        g_assert(dba);
#if LATER
        /* Requires having a file_id to summary_id table. See comment
           in db_summary_new().  Without such a table we can't control
           keeping historical data or not.  */
        dba->summary_id = 0;    /* force to increment */
#endif
        db_summary_add(wt->sig.summary);
        wt_remove(wt);
        if(wt->fp)
                fclose(wt->fp);
        g_free(wt);
        return;
}



/* Process the work list until the output queue is sufficiently full.
 */
void wt_process_queue(IOSBuf *ios)
{
        g_assert(ios);
        if(work_list)
                g_slist_foreach(work_list, wt_do_work, ios);
        return;
}



/* Roughly how much data we'd like to write to the output queue */
#define desired_queue_size 8192


/* Callback: actually do some work. */
static void wt_do_work(gpointer data, gpointer user_data)
{
        WorkTicket *wt;
        IOSBuf *ios;

        wt = (WorkTicket *)data;
        g_assert(wt);
        ios = (IOSBuf *)user_data;
        g_assert(ios);
        if(ios_buffer_size(ios) > desired_queue_size)
                return;
        
        switch(wt->status) {
        case wt_New:
                fprintf(stderr, "Unexpected new work ticket in wt_do_work.\n");
                break;

        case wt_AwaitingBlockList:
        case wt_AwaitingBlockConfirms:
        case wt_AwaitingBlockListConfirm:
                /* Nothing to do but wait */
                break;

        case wt_SendingBlocks:
                /* Send some more blocks */
                wt_send_blocks(wt, ios);
                break;

        case wt_SendingBlockRequests:
                /* Send some more block requests */
                wt_send_block_requests(wt, ios);
                break;

        default:
                fprintf(stderr,
                        "Unexpected case in wt_do_work: %d\n", wt->status);
                break;

        }
        return;
}



/* Send blocks until the output buffer is sufficiently full.
 */
static void wt_send_blocks(WorkTicket *wt, IOSBuf *ios)
{
        DBBlock *dbb;
        ProtoPutBlock pb;
        IOSBuf *ios1, *ios2;
        
        g_assert(wt);
        g_assert(ios);
        /* g_assert(wt->action == wt_Archive); */
        g_assert(wt->dbf);

        while((ios_buffer_size(ios) < desired_queue_size)
              && (wt->next_block_to_queue < wt->sig.blockList->len)) {
                dbb = g_ptr_array_index(wt->sig.blockList,
                                        wt->next_block_to_queue++);
                g_assert(dbb);
                if(dbb->in_new_cover && !dbb->remote_id) {
                        /* If dbb->remote_id, then it's already in the
                           remote block store, so don't send. */
                        cover_fill_block(dbb, wt->fp);
                        memset(&pb, 0, sizeof(pb));
                        pb.file_id = wt->dbf->file_id;
                        pb.block_id = dbb->local_id;
                        ios1 = ios_new();
                        ios_set_buffer(ios1, dbb->data, dbb->length);
                        ios2 = do_compress(ios1);
                        pb.ios = encrypt(ios2);
                        ios_free(ios1);
                        ios_free(ios2);
                        send_put_block(&pb);
                        ios_free(pb.ios);
                        wt->num_blocks_moved++;
                        g_free(dbb->data);
                }
        }
        if(wt->next_block_to_queue >= wt->sig.blockList->len)
                wt->status = wt_AwaitingBlockConfirms;
        return;
}



/* Send block requests until the output buffer is sufficiently full.
 */
static void wt_send_block_requests(WorkTicket *wt, IOSBuf *ios)
{
        DBBlock *dbb;
        ProtoGetBlock gb;
        
        g_assert(wt->action == wt_Extract);
        g_assert(wt);
        g_assert(ios);
        g_assert(wt->dbf);

        while((ios_buffer_size(ios) < desired_queue_size)
              && (wt->next_block_to_queue < wt->sig.blockList->len)) {
                dbb = g_ptr_array_index(wt->sig.blockList,
                                        wt->next_block_to_queue++);
                g_assert(dbb);
                memset(&gb, 0, sizeof(gb));
                gb.file_id = wt->dbf->file_id;
                gb.block_id = dbb->local_id;
                gb.archive_id = dbb->remote_id;
                send_get_block(&gb);
        }
        if(wt->next_block_to_queue >= wt->sig.blockList->len)
                wt->status = wt_AwaitingBlockConfirms;
        return;
}



#if DUMB
static int wt_check_if_done()
{
        int num = 0;
        
        g_slist_foreach(work_list, wt_count_remaining, &num);
        return num;
}



static void wt_count_remaining(gpointer data, gpointer user_data)
{
        WorkTicket *wt;
        int num;
        
        g_assert(data);
        g_assert(user_data);
        wt = (WorkTicket *)data;
        num = *(int *)user_data;
        num++;
        return;
}
#endif



/* Return true if the work list is full.  We don't enforce our idea of
   full, but we'd like our callers to respect it.
 */
int wt_queue_full()
{
        return g_slist_length(work_list) > WT_MAX_QUEUE_SIZE;
}



/* When we think we're done, it's nice to check to see that we are.
   If something's wrong, we should at least report it.  We don't
   actually do more than emit a warning of incomplete work.
 */
void wt_check_for_orphan_work()
{
        IOSBuf *ios;
        int num;
        int cmd;
        
        ios = io_write_buf();
        if(ios && (num = ios_buffer_size(ios))) {
                g_warning("%d bytes remain in write queue", num);
                cmd = (int)(*(char *)ios_at(ios, 0));
                proto_print_command(cmd);
        }
        if((num = g_slist_length(work_list)))
                g_warning("%d items remain in the work list", num);
        return;
}



/* Verify that a work ticket is likely to be valid. */
void wt_assert(WorkTicket *wt)
{
        g_assert(wt);
        g_assert(wt->fp);
        g_assert(wt->sig.blockList);
        g_assert(wt->sig.summary);
        g_assert(wt->dbf);
        g_assert(wt->next_block_to_queue < wt->sig.blockList->len);
        return;
}
