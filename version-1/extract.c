/*  extract.c
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


#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "covering.h"
#include "extract.h"
#include "filename.h"
#include "main.h"
#include "option.h"
#include "options.h"
#include "protocol.h"
#include "requests.h"
#include "summary.h"
#include "workticket.h"


static int extract_it(const char *file);
static int extract_file(DBFile *dbf, DBSummary *dbs);
static int extract_dir(const char *file);


int do_extract()
{
        guint i;
        ProtoBye pb;
        
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("begin do_extract");
        
        if(g_args->len == 0) {
                extract_it(NULL);
        } else {
                for(i = 0; i < g_args->len; i++)
                        extract_it((const char *)g_ptr_array_index(g_args, i));
        }

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("end do_extract");
        if(!wt_more()) {
                memset(&pb, 0, sizeof(pb));
                pb.error = pbe_OK;
                send_bye(&pb);
        }
        return 0;
}



/* ### To do: allow matching on regex's ### */
/*
  If file is NULL, extract all files.
*/
static int extract_it(const char *file)
{
        DBF_curs *curs;
        DBFile *dbf;
        DBSummary *dbs;
        
        if(file)
                g_message("Extract: %s", file);
        else
                g_message("Extracting all files.");
        
        curs = db_filename_curs_begin();
        while((dbf = db_filename_curs_next(curs))) {
                /* I trust file more than dbf->filename not to be
                   corrupt, so use its length. */
                if(!file || strncmp(dbf->filename, file, strlen(file)) == 0) {
                        if(file && int_option(kOption_verbose) & VERBOSE_FILES)
                                g_message("Archiving file '%s'.", file);
                        dbs = db_summary_fetch_last(dbf->file_id);
                        if(dbs)
                                extract_file(dbf, dbs);
                        else
                                g_warning("File is in database, but has "
                                          "no summary record: %s",
                                          dbf->filename);
                }
                cycle_main_loop();
        }
        return 0;
}



static int extract_file(DBFile *dbf, DBSummary *dbs)
{
        WorkTicket *wt;
        
        g_assert(dbf);
        g_assert(dbs);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                db_filename_display(dbf);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                db_summary_display(dbs);
        g_assert( (wt = new_work_ticket()) );
        wt->dbf = dbf;
        wt->sig.summary = dbs;
        /* ### Really should write to the real file. But there are
               details: we don't want to overwrite it, we might have
               to create directories, we should make arrangements to
               clean up in case the transfer is aborted so the user
               sees either a complete file or no file. ### */
        wt->fp = fopen("the-recovered-file", "w");
        g_assert(wt->fp);
        g_assert(( wt->sig.blockList = cover_new() ));
        wt->action = wt_Extract;
        /* ### If file has zero length, we could skip asking for the
               block list. ### */
        request_block_list(wt);
        wt->status = wt_AwaitingBlockList;
        wt_insert(wt);
        return 0;
}



/* ### To do: be able to extract directories, too. ### */
static int extract_dir(const char *file)
{
        g_assert(file);
        return 0;
}
