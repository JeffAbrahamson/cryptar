/*  db_misc.h
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

#ifndef __DB_MISC__
#define __DB_MISC__


#include <db.h>
#include <glib.h>


enum databases {
        kFilenames = 0,
        kSummary,
        kBlocks,
        kArchive,
        kConstants,
        kServerConstants,
        kKeys,

        kNumDatabases
};

struct ios_buf;

void db_error_exit();
void db_open(enum databases db);
DB *db_ptr(enum databases db);
void db_close(enum databases db);
void db_close_all();
guint32 db_get_next_index(enum databases db);
guint32 db_get_key_int(DBT key);
void db_put_key_int(DBT *key, guint32 id);
void db_open_client_dbs();
int db_open_server_dbs(int id, int pass);
void db_put(enum databases db, struct ios_buf *key, struct ios_buf *data);
void db_get(enum databases db, struct ios_buf *key, struct ios_buf *data);
void db_delete(enum databases db, struct ios_buf *key_in);
DBC *db_curs_begin(int db);
int db_curs_next(DBC *curs, struct ios_buf *key, struct ios_buf *data);
void db_curs_end(DBC *curs);
char *server_db_name();
char *client_db_name();
#endif
