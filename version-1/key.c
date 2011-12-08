/*  key.c
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
#include "key.h"
#include "ios.h"
#include "options.h"






DBKey *db_key_new(const guint32 key_id)
{
        DBKey *dbk;
        
        dbk = g_new(DBKey, 1);
        if(key_id)
                dbk->key_id = key_id;
        return dbk;
}



void db_key_free(DBKey *dbk)
{
        assert(dbk);
        if(dbk->key)
                g_free(dbk->key);
        g_free(dbk);
        return;
}



DBKey *db_key_add(const char *key_val)
{
        IOSBuf *key;
        IOSBuf *data;
        
        DBKey *dbk;

        key = ios_new();
        data = ios_new();

        ios_set_buffer(key, key_val, strlen(key_val));

        dbk = db_key_new(0);
        dbk->version = 0;
        dbk->key_id = db_get_next_index(kKeys);
        dbk->key = (char *)key_val;
        ios_append_int16(data, dbk->version);
        ios_append_string(data, dbk->key);

        db_put(kKeys, key, data);
        ios_free(key);
        ios_free(data);
        return dbk;
}



DBKey *db_key_fetch(const guint32 key_id)
{
        IOSBuf *key = NULL;
        IOSBuf *data = NULL;
        DBKey *dbk;
        
        key = ios_new();
        data = ios_new();
        ios_append_int32(key, key_id);
        db_get(kKeys, key, data);
        dbk = db_key_from_stream(data, key_id);
        ios_free(key);
        ios_free(data);
        return dbk;
}



DBKey *db_key_from_stream(IOSBuf *ios, const guint32 key_id)
{
        DBKey *dbk;

        assert(ios);
        if(!ios_buffer(ios))
                return NULL;
        dbk = db_key_new(key_id);
        dbk->version = ios_read_int16(ios);
        dbk->key = ios_read_str(ios);
        return dbk;
}




void db_key_display(DBKey *dbk)
{
        assert(dbk);
        printf("DBKey:\n");
        printf("  dbk->key_id = %d\n", dbk->key_id ? dbk->key_id : 0);
        printf("  dbk->key = %s\n", dbk->key);
        printf("  dbk->version = %d\n", dbk->version);
        return;
}



DBK_curs *db_key_curs_begin()
{
        return db_curs_begin(kKeys);
}




DBKey *db_key_curs_next(DBK_curs *curs)
{
        int len;
        guint32 key_id;
        DBKey *dbk;
        IOSBuf *key, *data;

        g_assert(curs);
        key = ios_new();
        data = ios_new();
        if(!db_curs_next(curs, key, data))
                return NULL;
        len = ios_buffer_size(key);
        g_assert(len > 0);
        key_id = ios_read_int32(key);
        dbk = db_key_from_stream(data, key_id);
        ios_free(key);
        ios_free(data);
        return dbk;
}



void db_key_curs_end(DBK_curs *curs)
{
        g_assert(curs);
        db_curs_end(curs);
        return;
}


