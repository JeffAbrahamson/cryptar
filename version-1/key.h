/*  key.h
 *  Copyright (C) 2003 by:
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


#ifndef __KEY__
#define __KEY__

#include <db.h>
#include <glib.h>

struct ios_buf;

typedef DBC DBK_curs;

typedef struct db_key {
        guint key_id;           /* primary key */
        char *key;              /* probably not the right data type */
        guint8 version;
} DBKey;


DBKey *db_key_new(const guint32 key);
void db_key_free(DBKey *dbk);
DBKey *db_key_add(const char *key_id);
DBKey *db_key_fetch(const guint32 key_id);
DBKey *db_key_from_stream(struct ios_buf *ios, const guint32 key_id);
void db_key_display(DBKey *dbk);
DBK_curs *db_key_curs_begin();
DBKey *db_key_curs_next(DBK_curs *curs);
void db_key_curs_end(DBK_curs *curs);

#endif
