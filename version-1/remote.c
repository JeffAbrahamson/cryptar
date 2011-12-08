/*  remote.c
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


#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "db_misc.h"
#include "ios.h"
#include "remote.h"




void db_archive_add(DBArchive *b)
{
        IOSBuf *key, *data;

        assert(b);
        key = ios_new();
        if(!b->archive_id)
                b->archive_id = db_get_next_index(kArchive);
        ios_append_int32(key, b->archive_id);

        data = ios_new();
        ios_append(data, &(b->version), sizeof(b->version));
        ios_append_int16(data, ios_buffer_size(b->ios));
        ios_append_ios(data, b->ios);

        db_put(kArchive, key, data);
        ios_free(key);
        ios_free(data);
        return;
}



DBArchive *db_archive_fetch(guint32 id)
{
        IOSBuf *key, *data;
        DBArchive *dba;

        key = ios_new();
        ios_append_int32(key, id);
        data = ios_new();
        db_get(kArchive, key, data);
        dba = db_archive_from_stream(data, id);
        ios_free(key);
        ios_free(data);
        return dba;
}



DBArchive *db_archive_from_stream(IOSBuf *ios, const guint32 id)
{
        DBArchive *a;
        guint32 length;
        void *data;
        
        assert(ios);
        if(!ios_buffer(ios))
                return NULL;
        a = g_new0(DBArchive, 1);
        a->archive_id = id;
        a->version = ((char *)(ios_read(ios, 1)))[0];
        length = ios_read_int16(ios);
        data = g_new(char, length);
        memmove(data,
                ios_read(ios, length),
                length);
        a->ios = ios_new();
        ios_set_buffer(a->ios, data, length);
        return a;
}


DBArchive *db_archive_new()
{
        DBArchive *dba;

        dba = g_new0(DBArchive, 1);
        return dba;
}


void db_archive_free(DBArchive *a)
{
        assert(a);
        if(a->ios) ios_free(a->ios);
        g_free(a);
        return;
}



void db_archive_display(DBArchive *a)
{
        assert(a);
        printf("DBArchive:\n");
        printf("  a->archive_id = %d\n", a->archive_id);
        printf("  a->version = %d\n", a->version);
        printf("  a->(length) = %d\n", ios_buffer_size(a->ios));
        printf("  a->(data) = <omitted>\n");
        return;
}




