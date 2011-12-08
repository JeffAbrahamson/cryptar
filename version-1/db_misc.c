/*  db_misc.c
 *  Copyright (C) 2002, 2003, 2004 by:
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
#include <db.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "db_misc.h"
#include "ios.h"
#include "option.h"
#include "options.h"
#include "prefs.h"
#include "syscall.h"



static DB *open_db(const char *filename, const char *db_name);
static char *archive_dir_name();
static char *generic_db_name(const char *name);
static char *db_name();


/* Not so aptly named, as we should call this before a normal exit,
   too. Exiting without closing the databases could potentially
   corrupt the databases.
 */
void db_error_exit()
{
        db_close_all();
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("at_exit handler db_error_exit");
        return;
}




static char *database_names[] = {
        "filename",
        "summary",
        "block",
        "archive",
        "constants",
        "server_constants",
        "keys"
};
static DB *database_ptrs[kNumDatabases];
static int db_init = 0;



void db_open(enum databases db)
{
        if(!db_init) {
                memset(database_ptrs, 0, sizeof(database_ptrs));
                db_init = 1;
        }
        database_ptrs[db] = open_db(db_name(), database_names[db]);
        assert(database_ptrs[db]);
        return;
}



void db_close(enum databases db)
{
        int ret;
        DB *dbp;

        dbp = database_ptrs[db];
        if(dbp && (ret = dbp->close(dbp, 0)))
                g_warning("Error closing database %s.\n", database_names[db]);
        database_ptrs[db] = 0;
        return;
}



void db_close_all()
{
        guint i;

        for(i = 0; i < kNumDatabases; i++) {
                if(database_ptrs[i]) {
                        db_close(i);
                        if(int_option(kOption_verbose) & VERBOSE_DB)
                                g_message("Closing database %d (%s)", i, database_names[i]);
                }
        }
        return;
}



/* To simplify things for the moment, open each database in a separate
   file. Using the same file requires locking and a shared DB_ENV. In
   brief, this stretches my current knowledge of Berkeley DB. The
   one_name stuff below is the implementation of this simplification.
 */
static DB *open_db(const char *filename, const char *db_name)
{
        char *one_name;         /* for now, use separate files, see
                                   above */
        int ret;
        DB *dbp;
        char *errfile;
        FILE *fp;
        DB_HASH_STAT sp;

        g_assert(filename);
        g_assert(db_name);
        one_name = g_strdup_printf("%s_%s", filename, db_name);
        if ((ret = db_create(&dbp, NULL, 0)) != 0) {
                g_warning("db_create: %s", db_strerror(ret));
                exit(-1);
        }
        /* ### Better would be to check if it exists and warn rather
               than blindly creating. ###
        */
#if DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR == 1
        if ((ret = dbp->open(dbp, NULL, /* filename */ one_name, db_name, DB_BTREE, DB_CREATE, 0600))
            != 0)
#else  /* DB_40 */
        if ((ret = dbp->open(dbp, /* filename */ one_name, db_name, DB_BTREE, DB_CREATE, 0600))
            != 0)
#endif
                {
                g_warning("db open error");
                dbp->err(dbp, ret, "%s", db_name);
                db_error_exit(dbp, ret);
                exit(1);
        }

        errfile = g_strdup_printf("%s_err.log", filename);
        fp = fopen(errfile, "a");
        if(fp)
                dbp->set_errfile(dbp, fp);
        else
                fprintf(stderr, "Failed to open error file %s\n", errfile);
        g_free(errfile);

        if(int_option(kOption_verbose) & VERBOSE_DB) {
                if(dbp->stat(dbp, &sp, 0)) {
                        g_message("Can't get statistics on database %s:%s",
                                filename, db_name);
                } else {
                        g_message("Statistics for %s:%s",
                                  filename, db_name);
                        g_message("  num unique keys = %dl",
                                  sp.hash_nkeys);
                        g_message("  num key/data pairs = %dl",
                                  sp.hash_ndata);
                        /*free(sp);*/
                }
        }
        g_free(one_name);
        return dbp;
}



DB *db_ptr(enum databases db)
{
        assert(database_ptrs[db]);
        return database_ptrs[db];
}



guint32 db_get_next_index(enum databases db)
{
        DB *dbp;
        DBT key, data;
        int ret;
        guint32 index;
        IOSBuf *ios_data;

        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("db_get_next_index, db=%d", db);
        if(int_option(kOption_server))
                dbp = db_ptr(kServerConstants);
        else
                dbp = db_ptr(kConstants);
        g_assert(dbp);

        ios_data = ios_new();

        /* Fetch old index, none means zero. */
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));
        db_put_key_int(&key, db);
        ret = dbp->get(dbp, NULL, &key, &data, 0);
        if(ret && ret != DB_NOTFOUND) {
                g_warning("db get error");
                dbp->err(dbp, ret, "DB->get");
                db_error_exit(dbp, ret);
                exit(1);
        }
        if(ret == DB_NOTFOUND) {
                if(int_option(kOption_verbose) & VERBOSE_DB)
                        g_message("db_get_next_index:   key not found, using 0");
                index = 0;
        } else {
                ios_set_buffer(ios_data, data.data, data.size);
                index = ios_read_int32(ios_data);
                g_assert(index);
                if(int_option(kOption_verbose) & VERBOSE_DB)
                        g_message("db_get_next_index:   key=%d", index);
        }
        index++;

        /* And update with new value */
        ios_reset(ios_data);
        ios_append_int32(ios_data, index);
        data.data = ios_buffer(ios_data);
        data.size = ios_buffer_size(ios_data);
        if((ret = dbp->put(dbp, NULL, &key, &data, 0))) {
                g_warning("db put error");
                dbp->err(dbp, ret, "DB->put");
                db_error_exit(dbp, ret);
                exit(1);
        }

        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("db_get_next_index:   new key=%d", index);
        ios_free(ios_data);
        return index;
}



guint32 db_get_key_int(DBT key)
{
        guint32 id;
        IOSBuf *ios;
        
        ios = ios_new();
        ios_set_buffer(ios, key.data, key.size);
        id = ios_read_int32(ios);
        ios_free(ios);
        return id;
}



void db_put_key_int(DBT *key, guint32 id)
{
        static IOSBuf *ios = NULL;

        assert(key);
        if(!ios)
                ios = ios_new();
        ios_reset(ios);
        ios_append_int32(ios, id);
        key->data = ios_buffer(ios);
        key->size = ios_buffer_size(ios);
        return;
}



void db_open_client_dbs()
{
        db_open(kFilenames);
        db_open(kSummary);
        db_open(kBlocks);
        db_open(kConstants);
}




/*
  Return 0 on success, non-zero on failure.
*/
int db_open_server_dbs(int id, int pass)
{
        db_open(kServerConstants);
        db_open(kArchive);
        if(const_get_int(PREF_ARCHIVE_ID) == id && const_get_int(PREF_ARCHIVE_PASS))
                return 0;
        return 1;
}



void db_put(enum databases db, IOSBuf *key_in, IOSBuf *data_in)
{
        DB *dbp;
        DBT key, data;
        int ret;
        
        g_assert(key_in);
        g_assert(data_in);
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));

        key.data = ios_buffer(key_in);
        key.size = ios_buffer_size(key_in);
        data.data = ios_buffer(data_in);
        data.size = ios_buffer_size(data_in);
        dbp = db_ptr(db);
        assert(dbp);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("db write: db=%d, len=%d", db, (int)data.size);
        if((ret = dbp->put(dbp, NULL, &key, &data, 0))) {
                g_warning("db put error");
                dbp->err(dbp, ret, "DB->put");
                db_error_exit(dbp, ret);
                exit(1);
        }
        ret = dbp->sync(dbp, 0);
        return;
}



void db_get(enum databases db, IOSBuf *key_in, IOSBuf *data_out)
{
        DB *dbp;
        DBT key, data;
        int ret;
        
        g_assert(key_in);
        g_assert(data_out);
        dbp = db_ptr(db);
        assert(dbp);
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));
        key.data = ios_buffer(key_in);
        key.size = ios_buffer_size(key_in);
        ret = dbp->get(dbp, NULL, &key, &data, 0);
        if(ret && ret != DB_NOTFOUND) {
                g_warning("db get error");
                dbp->err(dbp, ret, "DB->get");
                db_error_exit(dbp, ret);
                exit(1);
        }
        if(ret == DB_NOTFOUND)
                ios_reset(data_out);
        else
                ios_set_buffer(data_out, data.data, data.size);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("db read: db=%d, len=%d", db, (int)data.size);
        return;
}



void db_delete(enum databases db, IOSBuf *key_in)
{
        DB *dbp;
        DBT key, data;
        int ret;
        
        g_assert(key_in);
        dbp = db_ptr(db);
        assert(dbp);
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));
        key.data = ios_buffer(key_in);
        key.size = ios_buffer_size(key_in);
        ret = dbp->del(dbp, NULL, &key, 0);
        if(ret) {
                g_warning("db del: error, some data was not deleted");
                dbp->err(dbp, ret, "DB->del");
        }
        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("db del: db=%d", db);
        return;
}



DBC *db_curs_begin(int db)
{
        DB *dbp;
        DBC *curs;
        int ret;

        dbp = db_ptr(db);
        if((ret = dbp->cursor(dbp, NULL, &curs, 0))) {
                dbp->err(dbp, ret, "DB->cursor");
                g_warning("db_curs_begin: "
                        "Not sure what to do now, so quitting.");
                db_error_exit(dbp, ret);
                exit(1);
        }
        return curs;
}



int db_curs_next(DBC *curs, IOSBuf *ios_key, IOSBuf *ios_data)
{
        int ret;
        DBT key, data;

        g_assert(curs);
        g_assert(ios_key);
        g_assert(ios_data);
        if(!ios_key)
                ios_key = ios_new();
        if(!ios_data)
                ios_data = ios_new();
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));
        ret = curs->c_get(curs, &key, &data, DB_NEXT);
        if(ret == DB_NOTFOUND) {
                db_curs_end(curs);
                return 0;
        }
        if(ret) {
                curs->dbp->err(curs->dbp, ret, "DB->get");
                g_warning("db_curs_next: "
                        "Not sure what to do now, so quitting.");
                db_error_exit();
                exit(1);
        }
        ios_set_buffer(ios_key, key.data, key.size);
        ios_set_buffer(ios_data, data.data, data.size);
        return 1;
}



void db_curs_end(DBC *curs)
{
        int ret;

        g_assert(curs);
        if((ret = curs->c_close(curs))) {
                curs->dbp->err(curs->dbp, ret, "DB->get");
                g_warning("db_curs_end: "
                        "Not sure what to do now, so quitting.");
                db_error_exit(curs->dbp, ret);
                exit(1);
        }
        return;
}



static char *archive_dir_name()
{
        static gchar *archive_dir_ = NULL;
        
        const char *home;
        char *pref_dir;
        
        if(!archive_dir_) {
                home = getenv("HOME");
                if(!home) {
                        g_warning("$HOME is not defined. Using current directory.\n");
                        home = ".";
                }
                pref_dir = g_strdup_printf("%s/cryptar", home);
                assert(!do_mkdir(pref_dir, 0700));
                archive_dir_ = g_strdup_printf("%s/%s",
                                               pref_dir,
                                               char_option(kArchive_target));
                g_free(pref_dir);
                assert(!do_mkdir(archive_dir_, 0700));
        }
        
        return archive_dir_;
}



char *server_db_name()
{
        return generic_db_name("server");
}



char *client_db_name()
{
        return generic_db_name("client");
}



static char *generic_db_name(const char *name)
{
        const char *base_dir;
        char *dir, *dbname;

        g_assert(name);
        base_dir = archive_dir_name();
        g_assert(base_dir);
        dir = g_strdup_printf("%s/%s", base_dir, name);
        g_assert(dir);
        g_assert(!do_mkdir(dir, 0700));
        dbname = g_strdup_printf("%s/db", dir);
        g_free(dir);
        return dbname;
}



static char *db_name()
{
        if(int_option(kOption_server))
                return server_db_name();
        return client_db_name();
}

