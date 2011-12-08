/*  protocol.h
 *  Copyright (C) 2002-2003 by:
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

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <glib.h>


struct ios_buf;


enum ProtoHelloError {
        phe_OK = 0,
        phe_Not_now,
        phe_Disk_full,
        phe_Bad_pass,           /* id or password not valid */
        phe_Unknown
};

enum ProtoByeError {
        pbe_OK = 0,
        pbe_Disk_full,
        pbe_Going_down,
        pbe_Uknown
};


typedef struct proto_hello {
        guint8 version;
        guint32 archive_id;     /* server's name for this archive, zero if creating */
        guint32 archive_pass;   /* local's passphrase, always provide.
                                   This is only protection against
                                   accident, it's not a real secret
                                   passphrase. */
        guint8 create_archive;  /* non-zero to create an archive (and
                                   ignore zero archive_id, else zero
                                   archive_id indicates a ping and
                                   quit request.
                                */
        char *message;          /* NULL-terminated */
} ProtoHello;

typedef struct proto_hello_ack {
        guint8 version;
        enum ProtoHelloError error;
        guint32 archive_id;     /* server's name for this archive, zero if error */
        char *message;          /* NULL-terminated */
} ProtoHelloAck;

typedef struct proto_bye {
        guint8 version;
        enum ProtoByeError error;
} ProtoBye;

/* Put and Get are from the perspective of the client. */
typedef struct proto_put_block {
        guint32 file_id;
        guint32 block_id;
        struct ios_buf *ios;
} ProtoPutBlock;

typedef struct proto_put_block_ack {
        guint32 file_id;
        guint32 block_id;
        guint32 archive_id;
} ProtoPutBlockAck;

typedef struct proto_get_block {
        guint32 file_id;
        guint32 block_id;
        guint32 archive_id;
} ProtoGetBlock;

typedef struct proto_get_block_ack {
        guint32 file_id;
        guint32 block_id;
        struct ios_buf *ios;    /* on error == 0 */
} ProtoGetBlockAck;


int read_and_dispatch_commands(struct ios_buf *ios);
void proto_print_command(int cmd);
void send_hello(ProtoHello *h);
void send_hello_ack(ProtoHelloAck *ha);
void send_bye(ProtoBye *b);
void send_put_block(ProtoPutBlock *pb);
void send_put_ack(ProtoPutBlockAck *pba);
void send_get_ack(ProtoGetBlockAck *pga);
void send_get_block(ProtoGetBlock *pg);

#endif
