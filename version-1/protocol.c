/*  protocol.c
 *  Copyright (C) 2002-2004 by:
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


#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "archive.h"
#include "block.h"
#include "compress.h"
#include "constants.h"
#include "covering.h"
#include "db_misc.h"
#include "encryption.h"
#include "extract.h"
#include "filename.h"
#include "io.h"
#include "ios.h"
#include "main.h"
#include "option.h"
#include "options.h"
#include "prefs.h"
#include "protocol.h"
#include "queue.h"
#include "remote.h"
#include "requests.h"
#include "workticket.h"



/* Abstract the wire protocol, and provide a driver for handling input.
   
   These functions all return 0 on success, non-zero on failure.

   The receive_* functions marshal a stream into a structure, then
   call the corresponding accept_* function.  The accept_* functions
   actually do whatever needs to be done.
*/



static int read_and_dispatch_one_command(IOSBuf *ios);
static int receive_hello(IOSBuf *ios);
static int receive_hello_ack(IOSBuf *ios);
static int receive_bye(IOSBuf *ios);
static int receive_put_block(IOSBuf *ios);
static int receive_put_ack(IOSBuf *ios);
static int receive_get_ack(IOSBuf *ios);
static int receive_get_block(IOSBuf *ios);
static int accept_hello(ProtoHello *ph);
static int accept_hello_ack(ProtoHelloAck *pha);
static void do_accept_hello_ack(ProtoHelloAck *pha);
static int accept_put_block(ProtoPutBlock *pb);
static int accept_put_ack(ProtoPutBlockAck *pba);
static int accept_get_block(ProtoGetBlock *gb);
static void send_bad_get_ack(ProtoGetBlock *gb);
static int accept_get_ack(ProtoGetBlockAck *gba);
static void accept_get_ack_block(WorkTicket *wt, ProtoGetBlockAck *gba);
static void install_block(WorkTicket *wt, DBBlock *dbb);
static void accept_get_ack_block_list(WorkTicket *wt, ProtoGetBlockAck *gba);
static void start_archive(WorkTicket *wt);
static void start_extract(WorkTicket *wt);



enum proto_command {
        cmd_Hello,              /* greet a (remote) block store,
                                   request to log in */
        
        cmd_HelloAck,           /* the block store acknowledges the
                                   client, maybe provides some
                                   administrative information */

        cmd_Bye,                /* either side says it's done
                                   talking */

        cmd_PutBlock,           /* send a block of data to the block
                                   store */

        cmd_PutBlockAck,        /* acknowledge receipt of data at the
                                   block store, inform the client of
                                   the ID of the block for future
                                   reference */

        cmd_GetBlock,           /* request a block from the block
                                   store */

        cmd_GetBlockAck         /* provide a requested block from the
                                   block store */
};



int read_and_dispatch_commands(IOSBuf *ios)
{
        int ret;
        
        while(!(ret = read_and_dispatch_one_command(ios))) {
                if(ret < 0) {
                        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                                g_message("read_and_dispatch_commands returned < 0");
                        return -1;
                }
        }
        return 0;
}



/* Returns 0 if it was happy,
   1 if it couldn't do anything with the input stream,
   -1 if the stream has closed (cmd_Bye),
   -2 if the stream is garbled (probably have to give up)
*/
static int read_and_dispatch_one_command(IOSBuf *ios)
{
        IOSBuf *packet;
        int cmd;
        int packet_len, ret;
        
        g_assert(ios);
        packet = ios_new();
        if(ios_buffer_size(ios) < 5)
                return 1;       /* need an int32 (size) and an int8 (command) */
        if(ios_int32_at(ios, 0) > ios_buffer_size(ios) - 4)
                return 1;       /* need a full packet */
        packet_len = ios_read_int32(ios);
        g_assert(packet_len);   /* zero-length packets aren't legal */
        ios_set_buffer(packet, ios_read(ios, packet_len), packet_len);
        cmd = (int)(*(char *)ios_at(packet, 0));
        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                g_message("read_and_dispatch_one_command: got %d (length=%d)",
                          (int)cmd, packet_len);
        ret = -2;
        switch(cmd) {
        case cmd_Hello:
                ret = receive_hello(packet);
                break;
                
        case cmd_HelloAck:
                ret = receive_hello_ack(packet);
                break;
                
        case cmd_Bye:
                ret = receive_bye(packet);
                break;
                
        case cmd_PutBlock:
                ret = receive_put_block(packet);
                break;

        case cmd_PutBlockAck:
                ret = receive_put_ack(packet);
                break;

        case cmd_GetBlock:
                ret = receive_get_block(packet);
                break;
                
        case cmd_GetBlockAck:
                ret = receive_get_ack(packet);
                break;
                
        default:
                g_warning("Unexpected case in read_and_dispatch_one_command: %d",
                          cmd);
        }
        ios_free(packet);
        return ret;
}



void proto_print_command(int cmd)
{
        switch(cmd) {
        case cmd_Hello:
                g_message("  Looks like a cmd_Hello");
                break;
        case cmd_HelloAck:
                g_message("  Looks like a cmd_HelloAck");
                break;
        case cmd_Bye:
                g_message("  Looks like a cmd_Bye");
                break;
        case cmd_PutBlock:
                g_message("  Looks like a cmd_PutBlock");
                break;
        case cmd_PutBlockAck:
                g_message("  Looks like a cmd_PutBlockAck");
                break;
        case cmd_GetBlock:
                g_message("  Looks like a cmd_GetBlock");
                break;
        case cmd_GetBlockAck:
                g_message("  Looks like a cmd_GetBlockAck");
                break;
        default:
                g_warning(" Doesn't look like the beginning of a command block.");
        }
        return;
}



void send_hello(ProtoHello *h)
{
        IOSBuf *ios;
        guint8 c;
        
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("send_hello");
        g_assert(h);
        ios = ios_new();
        c = cmd_Hello;
        ios_append(ios, &c, 1);
        ios_append(ios, &h->version, 1);
        ios_append_int32(ios, h->archive_id);
        ios_append_int32(ios, h->archive_pass);
        ios_append(ios, &h->create_archive, 1);
        ios_append_string(ios, h->message);
        io_send_ios(ios);
        ios_free(ios);
        return;
}


void send_hello_ack(ProtoHelloAck *ha)
{
        IOSBuf *ios;
        guint8 c;
        
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("send_hello_ack");
        g_assert(ha);
        ios = ios_new();
        c = cmd_HelloAck;
        ios_append(ios, &c, 1);
        ios_append(ios, &ha->version, 1);
        ios_append_int16(ios, ha->error);
        ios_append_int32(ios, ha->archive_id);
        ios_append_string(ios, ha->message);
        io_send_ios(ios);
        ios_free(ios);
        return;
}


void send_bye(ProtoBye *b)
{
        IOSBuf *ios;
        guint8 c;
        
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("send_bye");
        g_assert(b);
        ios = ios_new();
        c = cmd_Bye;
        ios_append(ios, &c, 1);
        ios_append(ios, &b->version, 1);
        ios_append_int16(ios, b->error);
        io_no_more_input();
        io_send_ios(ios);
        ios_free(ios);
        io_no_more_output();
        return;
}


void send_put_block(ProtoPutBlock *pb)
{
        IOSBuf *ios;
        guint8 c;
        
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("send_put_block: file=%d, block=%d, length=%d",
                          pb->file_id, pb->block_id, ios_buffer_size(pb->ios));
        g_assert(pb);
        ios = ios_new();
        c = cmd_PutBlock;
        ios_append(ios, &c, 1);
        ios_append_int32(ios, pb->file_id);
        ios_append_int32(ios, pb->block_id);
        ios_append_int32(ios, ios_buffer_size(pb->ios));
        ios_append_ios(ios, pb->ios);
        io_send_ios(ios);
        ios_free(ios);
        return;
}



void send_put_ack(ProtoPutBlockAck *pba)
{
        IOSBuf *ios;
        guint8 c;

        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("send_put_ack, archive_id=%d", pba->archive_id);
        g_assert(pba);
        ios = ios_new();
        c = cmd_PutBlockAck;
        ios_append(ios, &c, 1);
        ios_append_int32(ios, pba->file_id);
        ios_append_int32(ios, pba->block_id);
        ios_append_int32(ios, pba->archive_id);
        io_send_ios(ios);
        ios_free(ios);
        return;
}



void send_get_block(ProtoGetBlock *pg)
{
        IOSBuf *ios;
        guint8 c;

        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("send_get_block");
        g_assert(pg);
        g_assert(pg->file_id);
        g_assert(pg->archive_id);
        ios = ios_new();
        c = cmd_GetBlock;
        ios_append(ios, &c, 1);
        ios_append_int32(ios, pg->file_id);
        ios_append_int32(ios, pg->block_id);
        ios_append_int32(ios, pg->archive_id);
        io_send_ios(ios);
        ios_free(ios);
        return;
}



void send_get_ack(ProtoGetBlockAck *pga)
{
        IOSBuf *ios;
        guint8 c;
        
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("send_get_ack");
        g_assert(pga);
        ios = ios_new();
        c = cmd_GetBlockAck;
        ios_append(ios, &c, 1);
        ios_append_int32(ios, pga->file_id);
        ios_append_int32(ios, pga->block_id);
        ios_append_int32(ios, ios_buffer_size(pga->ios));
        ios_append_ios(ios, pga->ios);
        io_send_ios(ios);
        ios_free(ios);
        return;
}



/* Server-side */
static int receive_hello(IOSBuf *ios)
{
        ProtoHello ph;
        char *buf;

        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("receive_hello");
        g_assert(ios);
        buf = ios_read(ios, 1);
        g_assert(buf[0] == cmd_Hello);

        ph.version = *(guint8 *)ios_read(ios, 1);
        ph.archive_id = ios_read_int32(ios);
        ph.archive_pass = ios_read_int32(ios);
        ph.create_archive = *(guint8 *)ios_read(ios, 1);
        ph.message = ios_read_str(ios);
        return accept_hello(&ph);
}



/* Client-side */
static int receive_hello_ack(IOSBuf *ios)
{
        ProtoHelloAck pha;
        char *buf;

        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("receive_hello_ack");
        g_assert(ios);
        buf = ios_read(ios, 1);
        g_assert(buf[0] == cmd_HelloAck);
        pha.version = *(guint8 *)ios_read(ios, 1);
        pha.error = ios_read_int16(ios);
        /* ### what if ios_read_str returns NULL?  We're trusting the
               input stream too much here. ### */
        pha.message = g_strdup(ios_read_str(ios));
        return accept_hello_ack(&pha);
}



/* Both client and server side */
static int receive_bye(IOSBuf *ios)
{
        ProtoBye pb;
        char *buf;
        
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("receive_bye");
        g_assert(ios);
        buf = ios_read(ios, 1);
        g_assert(buf[0] == cmd_Bye);
        pb.version = *(guint8 *)ios_read(ios, 1);
        pb.error = ios_read_int16(ios);
        if(pb.error != pbe_OK)
                g_message("Remote hung up with error status: %d\n", pb.error);
        
        /* ### Probably should flag somewhere that the remote has hung
               up, in case we are waiting for anything. Or at least
               check that we aren't waiting for anything
               anymore. ### */
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Remote has said goodbye.\n");

        if(int_option(kOption_server)) {
                /* The client sends a goodbye when it has no more to
                   say, but we may have more to respond. It can't know
                   that we're done responding until it gets our
                   goodbye. So we send one. */
                memset(&pb, 0, sizeof(pb));
                pb.error = pbe_OK;
                send_bye(&pb);
        }
        quit_main_loop();
        
        return 0;               /* Should always succeed */
}



static int receive_put_block(IOSBuf *ios)
{
        char *buf;
        ProtoPutBlock pb;
        ProtoPutBlockAck pba;
        guint32 length;
        
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("receive_put_block");
        g_assert(ios);
        buf = ios_read(ios, 1);
        g_assert(buf[0] == cmd_PutBlock);
        pb.file_id = ios_read_int32(ios);
        pb.block_id = ios_read_int32(ios);
        length = ios_read_int32(ios);
        pb.ios = ios_new();
        ios_set_buffer(pb.ios, ios_read(ios, length), length);

        pba.file_id = pb.file_id;
        pba.block_id = pb.block_id;
        pba.archive_id = accept_put_block(&pb);

        return 0;
}



static int receive_put_ack(IOSBuf *ios)
{
        ProtoPutBlockAck pba;
        char *buf;

        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("receive_put_ack");
        g_assert(ios);
        buf = ios_read(ios, 1);
        g_assert(( buf[0] = cmd_PutBlockAck ));
        pba.file_id = ios_read_int32(ios);
        pba.block_id = ios_read_int32(ios);
        pba.archive_id = ios_read_int32(ios);
        accept_put_ack(&pba);
        return 0;
}



static int receive_get_block(IOSBuf *ios)
{
        ProtoGetBlock gb;
        char *buf;
        
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("receive_get_block");
        g_assert(ios);
        buf = ios_read(ios, 1);
        g_assert(buf[0] == cmd_GetBlock);
        gb.file_id = ios_read_int32(ios);
        gb.block_id = ios_read_int32(ios);
        gb.archive_id = ios_read_int32(ios);
        accept_get_block(&gb);
        return 0;
}



static int receive_get_ack(IOSBuf *ios)
{
        char *buf;
        ProtoGetBlockAck gba;
        guint32 length;
        
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("receive_get_ack");
        g_assert(ios);
        buf = ios_read(ios, 1);
        g_assert(buf[0] == cmd_GetBlockAck);
        gba.file_id = ios_read_int32(ios);
        gba.block_id = ios_read_int32(ios);
        length = ios_read_int32(ios);
        gba.ios = ios_new();
        ios_set_buffer(gba.ios, ios_read(ios, length), length);

        accept_get_ack(&gba);
        return 0;
}



static int accept_hello(ProtoHello *ph)
{
        ProtoHelloAck pha;
        ProtoBye pb;
        
        g_assert(ph);
        g_assert(ph->message);

        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("hello: archive_id=%d, archive_pass=%d, msg=%s",
                          ph->archive_id, ph->archive_pass,
                          ph->message ? ph->message : "");

        memset(&pha, 0, sizeof(pha));
        pha.error = phe_OK;
        pha.message = "Server version 0.0 here.";
        pha.archive_id = ph->archive_id;

        memset(&pb, 0, sizeof(pb));
        pb.error = pbe_OK;

        if(ph->archive_id) {
                /* Normal server */
                if(db_open_server_dbs(ph->archive_id, ph->archive_pass)) {
                        pha.error = phe_Bad_pass;
                        send_hello_ack(&pha);
                        send_bye(&pb);
                        quit_main_loop();
                        return 0;
                }
                send_hello_ack(&pha);
        } else if(ph->create_archive) {
                /* Create archive, then continue as normal server */
                /*
                  Create an archive_id, create a database based on
                  that name, store in it the archive id and password.
                */
                create server db;
                store ph->archive_pass;
                send_hello_ack(&pha);
                send bye;
        } else {
                /* ping mode */
                /* Use fputs, since an evil remote could embed % codes
                   to exploit fprintf. */
                g_message("Ping: Message: %s", ph->message);
                send_hello_ack(&pha);
                send_bye(&pb);
                quit_main_loop();
        }
        return 0;
}



/* client-side */
static int accept_hello_ack(ProtoHelloAck *pha)
{
        g_assert(pha);
        g_assert(pha->message);
        g_assert(int_option(kOption_server) == 0);
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("hello_ack: msg=%s", pha->message);
        if(int_option(kOption_ping)) {
                g_message("Ping: %s", pha->message);
                return 0;
        }
#ifdef MAINTAINER_MODE
        if(int_option(kOption_self_test)) {
                g_message("Ping: %s", pha->message);
                return 0;
        }
#endif
        switch(pha->error) {
        case phe_OK:
                do_accept_hello_ack(pha);
                break;
                
        case phe_Not_now:
                g_message("The server reports that it is temporarily unavailable.");
                break;
                
        case phe_Disk_full:
                g_message("The server is unavailable now: it does not have adequate disk space.");
                break;
                
        case phe_Unknown:
                g_message("The server is unavailable for the moment, "
                          "but the reason is unclear.");
                break;
        default:
                g_warning("Unexpected case in accept_hello_ack: %d",
                          pha->error);
        }
        return 0;
}



static void do_accept_hello_ack(ProtoHelloAck *pha)
{
        g_assert(pha);
        if(!pha->archive_id) {
                g_error("Failed to receive a valid archive id from the server, but no error was indicated.\n\t(This shouldn't happen.)");
        }
        if(const_get_int(PREF_MUST_CREATE)) {
                const_put_int(PREF_MUST_CREATE, 0);
                const_put_int(PREF_ARCHIVE_ID, ph->archive_id);
        }
        if(int_option(kOption_backup)) {
                do_archive();
        } else if(int_option(kOption_extract)) {
                do_extract();
        } else if(int_option(kOption_create)) {
                ProtoBye pb;

                const_put_int(PREF_ARCHIVE_ID, pha->archive_id);
                memset(&pb, 0, sizeof(pb));
                pb.error = pbe_OK;
                send_bye(&pb);
        } else {
                g_warning("Not sure what action to take in do_accept_hello_ack()");
                g_message("Probably nothing will get done now.");
        }
        return;
}



/* Server-side */
static int accept_put_block(ProtoPutBlock *pb)
{
        DBArchive a;
        ProtoPutBlockAck pba;
        
        g_assert(pb);
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("accept_put_block: file=%d, block=%d, length=%d",
                          pb->file_id,
                          pb->block_id,
                          ios_buffer_size(pb->ios));
        a.archive_id = 0;
        a.version = 0;
        a.ios = pb->ios;
        db_archive_add(&a);

        pba.file_id = pb->file_id;
        pba.block_id = pb->block_id;
        pba.archive_id = a.archive_id;
        send_put_ack(&pba);
        
        return 0;
}



/* Client-side */
static int accept_put_ack(ProtoPutBlockAck *pba)
{
        WorkTicket *wt;
        ProtoBye pb;
        
        g_assert(pba);
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("accept_put_block_ack: file=%d, block=%d, archive=%d",
                          pba->file_id,
                          pba->block_id,
                          pba->archive_id);
        wt = wt_find(pba->file_id);
        if(!wt) {
                g_warning("Failed to find work ticket for a PutBlockAck: "
                          "file_id = %d.\n", pba->file_id);
                return 1;
        }
        g_assert(wt->action == wt_Archive);
        if(pba->block_id) {
                /* we successfully stored a block */
                DBBlock *dbb;

                dbb = wt_find_block(wt, pba->block_id);
                if(!dbb) {
                        g_warning("Failed to find block for PutBlockAck: "
                                  "file_id=%d, block_id=%d.\n",
                                  pba->file_id, pba->block_id);
                        return 1;
                }
                dbb->block_acked = 1;
                dbb->remote_id = pba->archive_id;
                if((--wt->num_blocks_moved == 0) &&
                   (wt->status == wt_AwaitingBlockConfirms)) {
                        /* Got all the blocks, so send the block list */
                        send_block_list(&wt->sig);
                        wt->status = wt_AwaitingBlockListConfirm;
                }
                db_block_add(dbb);
        } else {
                /* we successfully stored a block list */
                g_assert(wt->sig.summary);
                wt->sig.summary->block_list_remote_id = pba->archive_id;
                wt_finish(wt);
                if(!wt_more() && !wt_more_to_come()) {
                        memset(&pb, 0, sizeof(pb));
                        pb.error = pbe_OK;
                        send_bye(&pb);
                }
        }
        return 0;
}



/* Server-side */
static int accept_get_block(ProtoGetBlock *gb)
{
        DBArchive *a;
        ProtoGetBlockAck gba;
        
        g_assert(gb);
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("accept_get_block: file=%d, block=%d, archive=%d",
                          gb->file_id,
                          gb->block_id,
                          gb->archive_id);
        if(!gb->archive_id) {
                send_bad_get_ack(gb);
                return 0;
        }
        a = db_archive_fetch(gb->archive_id);
        if(!a) {
                send_bad_get_ack(gb);
                return 0;
        }
        gba.file_id = gb->file_id;
        gba.block_id = gb->block_id;
        gba.ios = a->ios;
        send_get_ack(&gba);
        db_archive_free(a);
        return 0;
}



/* Server-side: client requested block that doesn't exist */
static void send_bad_get_ack(ProtoGetBlock *gb)
{
        ProtoGetBlockAck gba;

        g_assert(gb);
        gba.file_id = gb->file_id;
        gba.block_id = gb->block_id;
        gba.ios = ios_new();    /* no data: client will understand */
        send_get_ack(&gba);
        ios_free(gba.ios);
        return;
}



/* Client-side */
static int accept_get_ack(ProtoGetBlockAck *gba)
{
        WorkTicket *wt;

        g_assert(gba);
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("accept_get_ack: file=%d, block=%d, length=%d",
                          gba->file_id,
                          gba->block_id,
                          ios_buffer_size(gba->ios));
        wt = wt_find(gba->file_id);
        if(!wt) {
                g_warning("Failed to find work ticket for a GetBlockAck: "
                          "file_id = %d.\n", gba->file_id);
                return 1;
        }
        if(gba->block_id) {
                /* we successfully retrieved a block */
                accept_get_ack_block(wt, gba);
        } else {
                accept_get_ack_block_list(wt, gba);
        }
        return 0;
}



static void accept_get_ack_block(WorkTicket *wt, ProtoGetBlockAck *gba)
{
        DBBlock *dbb;
        IOSBuf *ios1, *ios2;
        
        g_assert(wt);
        g_assert(gba);
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("accept_get_ack_block");
        if(!gba->ios || !ios_buffer_size(gba->ios)) {
                /* ### Surely we can give a better error message ### */
                g_warning("Server couldn't find block: file=%d, block=%d",
                          gba->file_id, gba->block_id);
                return;
        }
        dbb = wt_find_block(wt, gba->block_id);
        if(!dbb) {
                g_warning("Failed to find block for GetBlockAck: "
                          "file_id=%d, block_id=%d.\n",
                          gba->file_id, gba->block_id);
                return;
        }
        dbb->block_acked = 1;
        wt->num_blocks_moved--;
        ios1 = decrypt(gba->ios);
        ios2 = do_decompress(ios1, dbb->length);
        ios_free(gba->ios);
        ios_free(ios1);
        dbb->data = ios_buffer(ios2);
        dbb->length = ios_buffer_size(ios2);
        install_block(wt, dbb);
        ios_free(ios2);
        return;
}



static void install_block(WorkTicket *wt, DBBlock *dbb)
{
        int num;
        SCsum sc;

        g_assert(wt);
        g_assert(dbb);

        /* Check that it's the block we expect. */
        if(!SCsum_cmp(SCsum_buffer(dbb->data, dbb->length, sc), dbb->strong_checksum)) {
                g_warning("file=%s, offset=%d, block=%d: detected imposter.",
                          wt->dbf->filename, dbb->offset, dbb->local_id);
                return;
        }

        /* We got a block, install it. */
        if(fseek(wt->fp, dbb->offset, SEEK_SET)) {
                g_warning("fseek error in accept_get_ack_block");
                perror("fseek:");
                /* ### Are we cleaning up adequately? ### */
                return;
        }
        num = fwrite(dbb->data, dbb->length, sizeof(char), wt->fp);
        if(num < dbb->length)
                g_warning("accept_get_ack_block: successfully wrote only %d of %d.",
                          num, dbb->length);
        
        if(wt->num_blocks_moved == 0 && wt->status == wt_AwaitingBlockConfirms) {
                /* All blocks acked and installed, so remove from WorkTicket queue. */
                wt_remove(wt);
        }
}



/* We have received a stored block list.  Deserialize it, store it in
   the workticket, and queue the next logical step (whatever that
   turns out to be).
 */
static void accept_get_ack_block_list(WorkTicket *wt, ProtoGetBlockAck *gba)
{
        IOSBuf *ios;
        
        g_assert(wt);
        g_assert(gba);
        /* we successfully retrieved a block list */
        if(int_option(kOption_verbose) & VERBOSE_PROTOCOL)
                g_message("accept_get_ack_block_list");
        if(!gba->ios || !ios_buffer_size(gba->ios)) {
                /* ### Surely we can give a better error message ### */
                g_warning("Server couldn't find block list: file=%d",
                          gba->file_id);
                return;
        }
        ios = decrypt(gba->ios);
        ios_free(gba->ios);
        gba->ios = do_decompress(ios, wt->sig.summary->block_list_length);
        ios_free(ios);
        if(wt->sig.blockList) {
                if(wt->sig.blockList->len > 0)
                        g_warning("accept_get_block_list: old block list had non-zero length (%d)", wt->sig.blockList->len);
                g_ptr_array_free(wt->sig.blockList, 0);
        }
        wt->sig.blockList = block_list_from_stream(gba->ios);
        g_assert(wt->sig.blockList);
        ios_free(gba->ios);
        if(int_option(kOption_backup))
                start_archive(wt);
        else if(int_option(kOption_extract))
                start_extract(wt);
        else
                g_error("Neither archive nor recovery, yet retrieved a block list.");
        return;
}




/* We just received a new block list, and we are archiving files.  So
   compute the edit script (the new cover) and set the workticket's
   status so that blocks will be sent to the remote store.  Save block
   list when we are done.
 */
static void start_archive(WorkTicket *wt)
{
        wt_assert(wt);

        /* ### Questions:

               - How is work queue handled vis-a-vis storing local DBBlocks?
                 (Answer: we should do it before queuing.)

           ###
         */
        cover_update_remote(wt->fp, &wt->sig);
        cover_save_local(wt->sig.blockList);

        wt->status = wt_SendingBlocks;
        wt->next_block_to_queue = 0;
        io_write_reg();
        return;
}



static void start_extract(WorkTicket *wt)
{
        /* some day check for local file, which will save network time */
        int have_local_file = 0;
        
        wt_assert(wt);
        if(have_local_file) {
                g_error("We don't know what to do if we have the local file.");
        } else {
                /* Local file is not available, so get all blocks from
                   remote.
                */
                wt->status = wt_SendingBlockRequests;
                io_write_reg();
        }
        return;
}



