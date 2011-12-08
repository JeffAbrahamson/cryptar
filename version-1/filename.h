/*  filename.h
 *  Copyright (C) 2002-2003 by:
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


#ifndef __FILENAME__
#define __FILENAME__

#include <db.h>
#include <glib.h>

struct ios_buf;

typedef DBC DBF_curs;

typedef struct db_file {
        char *filename;         /* primary key */
        guint32 file_id;
        guint8 version;
} DBFile;


DBFile *db_filename_new(const char *filename);
void db_filename_free(DBFile *dbf);
DBFile *db_filename_add(const char *filename);
DBFile *db_filename_fetch(const char *name);
DBFile *db_filename_from_stream(struct ios_buf *ios, const char *name);
DBFile *db_filename_fetch_by_id(guint32 id);
void db_filename_display(DBFile *dbf);
DBF_curs *db_filename_curs_begin();
DBFile *db_filename_curs_next(DBF_curs *curs);
void db_filename_curs_end(DBF_curs *curs);

#endif
