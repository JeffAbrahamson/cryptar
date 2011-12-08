/*  archive.c
 *  Copyright (C) 2002, 2003, 2004 by:
 *  Jeff Abrahamson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include <assert.h>
#include <errno.h>
#include <ftw.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

#include "archive.h"
#include "checksum.h"
#include "covering.h"
#include "db_misc.h"
#include "filename.h"
#include "io.h"
#include "ios.h"
#include "main.h"
#include "option.h"
#include "options.h"
#include "prefs.h"
#include "protocol.h"
#include "requests.h"
#include "summary.h"
#include "workticket.h"


static int archive_it(const char *file, const  struct  stat  *sb,  int  flag);
static int archive_file(const char *file, const struct stat *sb);
static int archive_dir(const char *file, const struct stat *sb);
static FILE *open_if_changed(const char *file, const struct stat *sb, DBSummary *dba, gboolean);


#define MAX_BUF 1000




/* Main routine for archiving a file.  The argument processing code
   (parse_options) will have determined what database we're working
   with.  We look up in the database to find out what files to
   archive, then queue all the work.
 */
int do_archive()
{
        char *to_archive_list;
        char **to_archive;
        int i;
        const int ftw_depth = 20;
        ProtoBye pb;

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("begin do_archive");
        wt_working(1);
        to_archive_list = pref_get_value(PREF_TO_ARCHIVE);
        if(!to_archive_list) {
                fprintf(stderr, "Nothing to archive.\n");
                memset(&pb, 0, sizeof(pb));
                pb.error = pbe_OK;
                send_bye(&pb);
                return 0;
        }

        to_archive = g_strsplit(to_archive_list, " ", 0);
        g_free(to_archive_list);
        for(i = 0; to_archive[i]; i++) {
                ftw(to_archive[i], archive_it, ftw_depth);
        }
        wt_working(0);
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("end do_archive");
        if(!wt_more()) {
                memset(&pb, 0, sizeof(pb));
                pb.error = pbe_OK;
                send_bye(&pb);
        }
        return 0;
}



/* Main routine for pinging a remote store.  argument processing code
   (parse_options) will have determined what database we're working
   with.  We look up in the database to find out the server, then send
   a message to confirm that we can log in.
 */
int do_ping()
{
        ProtoHello ph;
        
        memset(&ph, 0, sizeof(ph));
        /* archive_id and create equal to zero will tell server to
           ping and quit.
        */
        ph.message = "Ping from client version 0.0";

        send_hello(&ph);        /* id, pass, and create all zero indicates ping */
        run_main_loop();
        io_close_io();
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Ping event loop terminated.");
        
        return 0;
}



/* Callback from do_archive, ftw: return 0 to continue, non-zero to stop iteration */
static int archive_it(const char *file, const  struct  stat  *sb,  int  flag)
{
        int ret = 0;

        while(wt_queue_full())
                cycle_main_loop();
        switch(flag) {
        case FTW_F:
                /* normal file */
                if(int_option(kOption_verbose) & VERBOSE_FILES)
                        g_message("Archiving file '%s'.", file);
                ret = archive_file(file, sb);
                break;
        case FTW_D:
                /* directory */
                if(int_option(kOption_verbose) & VERBOSE_FILES)
                        g_message("Archiving directory '%s'.", file);
                ret = archive_dir(file, sb);
                break;
        case FTW_DNR:
                /* directory which can't be read */
                if(int_option(kOption_verbose) & VERBOSE_FILES)
                        g_message("Skipping unreadable directory %s.", file);
                break;
        case FTW_NS:
                /* not a sym link, but stat failed */
                if(int_option(kOption_verbose) & VERBOSE_FILES)
                        g_message("Skipping unreadable entry %s.", file);
                break;
        case FTW_SL:
                if(int_option(kOption_verbose) & VERBOSE_FILES)
                        g_message("Found a sym link, not following: %s.", file);
                /* I think ftw will follow sym links, unfortunately. */
                break;
        }
        if(!is_main_loop_running())
                ret = 1;
        return ret;
}



/* We'll end up here for each file we want to archive.  Here we
   determine if anything needs to be done (we may want it archived and
   find it already is up to date in the archive).  If work needs to be
   done, we'll queue it up.
 */
static int archive_file(const char *file, const struct stat *sb)
{
        DBFile *dbf;
        DBSummary *dbs = 0;
        gboolean new_file;
        FILE *fp;
        WorkTicket *wt;
        
        assert(file);
        assert(sb);

        new_file = 0;
        if(!(dbf = db_filename_fetch(file))) {
                dbf = db_filename_add(file);
                dbs = db_summary_new(dbf, sb);
                new_file = 1;
        } else if(!(dbs = db_summary_fetch_last(dbf->file_id))) {
                if(int_option(kOption_verbose) & VERBOSE_FILES)
                        g_message("Can't find last signature for file %s. "
                                  "Assume it's a new file.", file);
                dbs = db_summary_new(dbf, sb);
                new_file = 1;
        }
        fp = open_if_changed(file, sb, dbs, new_file);
        if(!fp)
                return 0;       /* nothing to do if no change */

        /* Something changed, so we'll need a new DBSummary record */
        dbs->summary_time = time(0);
        dbs->mod_time = sb->st_mtime;
        dbs->file_length = sb->st_size;
        dbs->inode_number = sb->st_ino;
        dbs->permissions = sb->st_mode;

        /* Set (and validate) a new workticket */
        g_assert(( wt = new_work_ticket() ));
        g_assert(( wt->dbf = dbf ));
        g_assert(( wt->sig.summary = dbs ));
        g_assert(( wt->fp = fp ));
        g_assert(( wt->sig.blockList = cover_new() ));
        wt->action = wt_Archive;

        if(new_file) {
                /* If it's a new file, we don't have to wait for the
                   block list to be returned from the server. */
                cover_update_remote(fp, &wt->sig);
                cover_save_local(wt->sig.blockList);
                wt->status = wt_SendingBlocks;
                /* We only insert here. It would be nice to queue up
                   all the blocks as well, but this isn't the place to
                   manage space constraints in the write queue. (The
                   file could be very large relative to the space we
                   have available to us.) The write queue will fetch
                   work when it's ready.
                 */
        } else {
                /* If the file is already in the archive, we have to
                   wait around for the block list. So queue it and
                   move on. */
                request_block_list(wt);
                wt->status = wt_AwaitingBlockList;
        }
        wt_insert(wt);
        cycle_main_loop();
        return 0;
}




/* We'll end up here for each directory we want to archive.  Here we
   determine if anything needs to be done (we may want it archived and
   find it already is up to date in the archive).  If work needs to be
   done, we'll queue it up.

   ### This isn't implemented yet.  Archiving a directory is mostly
       about remembering its permission settings. ###
 */
static int archive_dir(const char *file, const struct stat *sb)
{
        assert(file);
        assert(sb);
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_warning("Unimplimented: archive_dir: %s", file);
        return 0;
}



/* Open the file if we're going to need to read from it.
 */
static FILE *open_if_changed(const char *file, const struct stat *sb, DBSummary *dbs, gboolean open_anyway)
{
        char changed = 0;
        SCsum hash;
        FILE *fp;
        
        assert(file);
        assert(sb);
        assert(dbs);

        if(((guint32)sb->st_size != dbs->file_length) ||
           ((guint32)sb->st_mtime != dbs->mod_time) ||
           open_anyway)
                changed = 1;

        if(changed || pref_get_int_value_silent_default(PREF_ALWAYS_COMPARE_HASH, "1")) {
                if((fp = fopen(file, "r")) == NULL) {
                        perror("Failed to open file");
                        g_message("  File='%s'", file);
                        exit(1);        /* #### Temp #### */
                }
                SCsum_file(fp, hash);
                if(memcmp(hash, dbs->file_sha1, SCsumLength) == 0) {
                        fclose(fp);
                        return NULL;
                }
                g_memmove(dbs->file_sha1, hash, SCsumLength);
                return fp;
        }
        return NULL;        
}

