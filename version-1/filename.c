/*  filename.c
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


#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "checksum.h"
#include "db_misc.h"
#include "filename.h"
#include "ios.h"
#include "options.h"






DBFile *db_filename_new(const char *filename)
{
        DBFile *dbf;
        
        dbf = g_new(DBFile, 1);
        if(filename)
                dbf->filename = g_strdup(filename);
        return dbf;
}



void db_filename_free(DBFile *dbf)
{
        assert(dbf);
        if(dbf->filename)
                g_free(dbf->filename);
        g_free(dbf);
        return;
}



DBFile *db_filename_add(const char *filename)
{
        IOSBuf *key;
        IOSBuf *data;
        
        DBFile *dbf;

        key = ios_new();
        data = ios_new();

        ios_set_buffer(key, filename, strlen(filename));

        dbf = db_filename_new(filename);
        dbf->version = 0;
        dbf->file_id = db_get_next_index(kFilenames);
        ios_append_int16(data, dbf->version);
        ios_append_int32(data, dbf->file_id);

        db_put(kFilenames, key, data);
        ios_free(key);
        ios_free(data);
        return dbf;
}



DBFile *db_filename_fetch(const char *name)
{
        IOSBuf *key;
        IOSBuf *data;
        DBFile *dbf;
        
        key = ios_new();
        data = ios_new();
        ios_set_buffer(key, name, strlen(name));
        db_get(kFilenames, key, data);
        dbf = db_filename_from_stream(data, name);
        ios_free(key);
        ios_free(data);
        return dbf;

}



DBFile *db_filename_from_stream(IOSBuf *ios, const char *name)
{
        DBFile *dbf;

        assert(ios);
        if(!ios_buffer(ios))
                return NULL;
        dbf = db_filename_new(name);
        dbf->version = ios_read_int16(ios);
        dbf->file_id = ios_read_int32(ios);
        return dbf;
}




DBFile *db_filename_fetch_by_id(guint32 id)
{
        assert(id);
        
        fprintf(stderr, "db_filename_fetch_by_id() not yet implemented.\n");
        return NULL;
}



void db_filename_display(DBFile *dbf)
{
        assert(dbf);
        printf("DBFile:\n");
        printf("  dbf->filename = %s\n", dbf->filename ? dbf->filename : "");
        printf("  dbf->file_id = %d\n", dbf->file_id);
        printf("  dbf->version = %d\n", dbf->version);
        return;
}



DBF_curs *db_filename_curs_begin()
{
        return db_curs_begin(kFilenames);
}




DBFile *db_filename_curs_next(DBF_curs *curs)
{
        int len;
        char *name;
        DBFile *dbf;
        IOSBuf *key, *data;

        g_assert(curs);
        key = ios_new();
        data = ios_new();
        if(!db_curs_next(curs, key, data))
                return NULL;
        len = ios_buffer_size(key);
        g_assert(len > 0);
        name = g_strndup(ios_read(key, len), len);
        dbf = db_filename_from_stream(data, name);
        g_free(name);
        ios_free(key);
        ios_free(data);
        return dbf;
}



void db_filename_curs_end(DBF_curs *curs)
{
        g_assert(curs);
        db_curs_end(curs);
        return;
}


