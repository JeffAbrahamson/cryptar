/*  list.c
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


#include <db.h>
#include <glib.h>
#include <stdio.h>
#include <time.h>

#include "config.h"

#include "filename.h"
#include "list.h"
#include "options.h"
#include "summary.h"

static void list_display_file(DBFile *dbf, DBSummary *dbs);



int do_list()
{
        DBF_curs *curs;
        DBFile *dbf;
        DBSummary *dbs;

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("begin do_list");
        /* ### We should pay attention to char_option(kList_pattern). ### */
        curs = db_filename_curs_begin();
        while((dbf = db_filename_curs_next(curs))) {
                dbs = db_summary_fetch_last(dbf->file_id);
                list_display_file(dbf, dbs);
        }
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("end do_list");
        return 0;
}



static void list_display_file(DBFile *dbf, DBSummary *dbs)
{
        g_assert(dbf);
        g_assert(dbs);
        printf("%s", dbf->filename);
        if(dbs && (int_option(kOption_verbose) & VERBOSE_FILES)) {
                printf(" (%d bytes)\n    backed up %s",
                       dbs->file_length, asctime(localtime((const time_t *)&dbs->summary_time)));
                printf("    modified %s", asctime(localtime((const time_t *)&dbs->mod_time)));
        } else
                printf("\n");
        return;
}






