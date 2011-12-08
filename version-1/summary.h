/*  summary.h
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


#ifndef __SUMMARY__
#define __SUMMARY__

#include <glib.h>

#include "checksum.h"

struct db_file;
struct stat;

typedef struct db_summary {
        guint32 summary_id;     /* primary key */
        guint8 version;
        guint32 file_id;
        guint32 summary_time;
        guint32 mod_time;
        guint32 file_length;
        guint32 inode_number;
        guint32 permissions;
        SCsum file_sha1;
        SCsum block_list_sha1;
        guint32 block_list_length;
        guint32 block_list_remote_id;
} DBSummary;

typedef struct signature {
        DBSummary *summary;     /* the summary signature entry */
        GPtrArray *blockList;   /* an array of DBBlock's */
} Signature;


void db_summary_add(DBSummary *a);
DBSummary *db_summary_fetch(guint32 id);
DBSummary *db_summary_from_stream(struct ios_buf *ios, const guint32 id);
DBSummary *db_summary_fetch_last(guint32 id);
DBSummary *db_summary_new(struct db_file *dbf, const struct stat *sb);
void db_summary_free(DBSummary *);
void db_summary_display(DBSummary *dbs);

#endif
