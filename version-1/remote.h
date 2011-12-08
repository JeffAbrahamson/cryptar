/*  remote.h
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


#ifndef __REMOTE__
#define __REMOTE__

struct ios_buf;

typedef struct db_archive {
        guint32 archive_id;
        guint8 version;
        struct ios_buf *ios;
} DBArchive;

void db_archive_add(DBArchive *b);
DBArchive *db_archive_fetch(guint32 id);
DBArchive *db_archive_new();
void db_archive_free(DBArchive *a);
void db_archive_display(DBArchive *a);
DBArchive *db_archive_from_stream(struct ios_buf *ios, const guint32 id);

#endif
