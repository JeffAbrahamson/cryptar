/*  summary.c
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
#include "summary.h"



static void db_summary_to_stream(IOSBuf *ios, DBSummary *a);



void db_summary_add(DBSummary *a)
{
        IOSBuf *key, *data;

        assert(a);
        key = ios_new();
        if(!a->summary_id)
                a->summary_id = db_get_next_index(kSummary);
        ios_append_int32(key, a->summary_id);

        data = ios_new();
        db_summary_to_stream(data, a);
        db_put(kSummary, key, data);
        ios_free(key);
        ios_free(data);
        return;
}



/* Look up the DBSummary with given ID.  If it doesn't exist in the
   database, return NULL.  If it exists, allocate, initialize, and
   return a DBSummary structure.
 */
DBSummary *db_summary_fetch(guint32 id)
{
        IOSBuf *key, *data;
        DBSummary *dbs;
        
        key = ios_new();
        data = ios_new();
        ios_append_int32(key, id);
        db_get(kSummary, key, data);
        if(ios_buffer_size(data) == 0)
                return NULL;
        dbs = db_summary_from_stream(data, id);
        ios_free(key);
        ios_free(data);
        return dbs;
}



static void db_summary_to_stream(IOSBuf *ios, DBSummary *a)
{
        g_assert(ios);
        g_assert(a);
        
        ios_append(ios, &(a->version), sizeof(a->version));
        ios_append_int32(ios, a->file_id);
        ios_append_int32(ios, a->summary_time);
        ios_append_int32(ios, a->mod_time);
        ios_append_int32(ios, a->file_length);
        ios_append_int32(ios, a->inode_number);
        ios_append_int32(ios, a->permissions);
        ios_append(ios, a->file_sha1, SCsumLength);
        ios_append(ios, a->block_list_sha1, SCsumLength);
        ios_append_int32(ios, a->block_list_length);
        ios_append_int32(ios, a->block_list_remote_id);
        return;
}



DBSummary *db_summary_from_stream(IOSBuf *ios, const guint32 id)
{
        DBSummary *dbs;

        assert(ios);
        dbs = g_new(DBSummary, 1);
        dbs->summary_id = id;
        
        dbs->version = ((char *)(ios_read(ios, 1)))[0];
        dbs->file_id = ios_read_int32(ios);
        dbs->summary_time = ios_read_int32(ios);
        dbs->mod_time = ios_read_int32(ios);
        dbs->file_length = ios_read_int32(ios);
        dbs->inode_number = ios_read_int32(ios);
        dbs->permissions = ios_read_int32(ios);
        g_memmove(dbs->file_sha1, ios_read(ios, SCsumLength), SCsumLength);
        g_memmove(dbs->block_list_sha1, ios_read(ios, SCsumLength), SCsumLength);
        dbs->block_list_length = ios_read_int32(ios);
        dbs->block_list_remote_id = ios_read_int32(ios);
        return dbs;        
}



DBSummary *db_summary_fetch_last(guint32 id)
{
        DB *dbp;

        dbp = db_ptr(kSummary);
        return db_summary_fetch(id); /* ### Currently we just fetch
                                        based on index (id), but we
                                        eventually should support
                                        fetching the most recent
                                        summary for a given file
                                        id. ### */
}



DBSummary *db_summary_new(DBFile *dbf, const struct stat *sb)
{
        DBSummary *dbs;

        assert(dbf);
        assert(dbf->filename);
        assert(sb);
        
        dbs = g_new(DBSummary, 1);

        dbs->summary_id = dbf->file_id; /* should be 0, then assigned
                                           later. But that will
                                           require having a separate
                                           file_id to summary_id
                                           table. This just means we
                                           can't keep historical
                                           information for now. */
        dbs->version = 0;
        dbs->file_id = dbf->file_id;
        dbs->summary_time = time(0);
        dbs->mod_time = sb->st_mtime;
        dbs->file_length = sb->st_size;
        dbs->inode_number = sb->st_ino;
        dbs->permissions = sb->st_mode;
        memset(dbs->file_sha1, 0, sizeof(SCsum));
        memset(dbs->block_list_sha1, 0, sizeof(SCsum));
        dbs->block_list_length = 0;
        dbs->block_list_remote_id = 0;
        
        return dbs;
}



void db_summary_free(DBSummary *dbs)
{
        assert(dbs);
        g_free(dbs);
        return;
}



void db_summary_display(DBSummary *dbs)
{
        assert(dbs);
        printf("DBSummary:\n");
        printf("  dbs->summary_id = %d", dbs->summary_id);
        printf("  dbs->version = %d", dbs->version);
        printf("  dbs->file_id = %d", dbs->file_id);
        printf("  dbs->summary_time = %d", dbs->summary_time);
        printf("  dbs->mod_time = %d", dbs->mod_time);
        printf("  dbs->file_length = %d", dbs->file_length);
        printf("  dbs->inode_number = %d", dbs->inode_number);
        printf("  dbs->permssions = o%od", dbs->permissions);
        printf("  dbs->block_list_length = %d", dbs->block_list_length);
        printf("  dbs->block_list_remote_id = %d", dbs->block_list_remote_id);
        printf("\ndbs->file_sha1 = ");
        SCsum_print(dbs->file_sha1);
        printf("\ndbs->block_list_sha1 = ");
        SCsum_print(dbs->block_list_sha1);
        printf("\n");
        return;
}

