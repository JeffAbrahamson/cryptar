/*  cryptar-db.c
 *  Copyright (C) 2002, 2003 by:
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
#include <getopt.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "db_misc.h"
#include "filename.h"
#include "ios.h"
#include "options.h"
#include "prefs.h"
#include "protocol.h"
#include "remote.h"
#include "summary.h"


int main(int argc,char *argv[]);

static int parse_cdb_options(int argc, char **argv);
static void cdb_usage();
static void find_by_id();
static void display_all();


enum actions {
        kFind_By_ID,
        kShow_All_IDs,
        kShow_All
};

static int the_database = -1;
static int the_action = -1;
typedef void *(*FetchFunc)(void *);
static FetchFunc the_fetch_func = NULL;
typedef void(*DisplayFunc)(void *);
static DisplayFunc the_display_func = NULL;
char *the_key = NULL;


int main(int argc,char *argv[])
{
        g_atexit(db_error_exit);
	if(parse_cdb_options(argc, argv)) {
		return 1;
	}
        if(the_database < 0) {
                fprintf(stderr, "Please select a database.\n");
                cdb_usage();
                return 1;
        }
        if(the_action < 0) {
                fprintf(stderr, "Please select an action.\n");
                cdb_usage();
                return 1;
        }
        printf("DB = %d, action = %d.\n", the_database, the_action);
        switch(the_action) {
        case kFind_By_ID:
                find_by_id();
                break;
        case kShow_All_IDs:
        case kShow_All:
                display_all();
                break;
        default:
                fprintf(stderr, "Unexpected case in main:switch(the_action): %d\n",
                        the_action);
        }
        return 0;
}




static struct option long_options[] = {
        {"database", required_argument, NULL, 'd'},
        {"action", required_argument, NULL, 'a'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
};

static int parse_cdb_options(int argc, char **argv)
{
        int ret = 0;
        int no_archive = 0;
        int c;
        
        while (c = getopt_long (argc, argv, "d:a:h", long_options, 0),
               c != EOF) {

                switch (c) {
                case 'd':
                        switch(*optarg) {
                        case 'f': the_database = kFilenames;
                                the_fetch_func = (FetchFunc)db_filename_fetch;
                                the_display_func = (DisplayFunc)db_filename_display;
                                break;
                        case 's': the_database = kSummary;
                                the_fetch_func = (FetchFunc)db_summary_fetch;
                                the_display_func = (DisplayFunc)db_summary_display;
                                break;
                        case 'b': the_database = kBlocks;
                                the_fetch_func = (FetchFunc)db_block_fetch;
                                the_display_func = (DisplayFunc)db_block_display;
                                break;
                        case 'a': the_database = kArchive;
                                the_fetch_func = (FetchFunc)db_archive_fetch;
                                the_display_func = (DisplayFunc)db_archive_display;
                                set_int_option(kOption_server, 1);
                                break;
                        case 'c': the_database = kConstants;
                                /*the_fetch_func = (FetchFunc)db_constant_fetch;*/
                                /*the_display_func = (DisplayFunc)db_constant_display;*/
                                break;
                        case 'e': the_database = kServerConstants;
                                /*the_fetch_func = (FetchFunc)db_constant_fetch;*/
                                /*the_display_func = (DisplayFunc)db_constant_display;*/
                                set_int_option(kOption_server, 1);
                                break;
                        case 'k': the_database = kKeys;
                                the_fetch_func = NULL;
                                the_display_func = NULL;
                                break;
                        default:
                                fprintf(stderr, "Bad database name: %c.\n",
                                        *optarg);
                                ret = 1;
                                break;
                        }
                        break;
                        
                case 'a':
                        switch(*optarg) {
                        case 'f': the_action = kFind_By_ID; break;
                        case 'l': the_action = kShow_All_IDs; break;
                        case 's': the_action = kShow_All; break;
                        default:
                                fprintf(stderr, "Bad action: %c.\n",
                                        *optarg);
                                ret = 1;
                                break;
                        }
                        break;

                case 'h':
                        cdb_usage();
                        ret = 1;
                        no_archive = 1;
                        break;
                        
                case ':':
                        printf("Required argument missing.\n");
                        ret = 1;
                        break;
                        
                case '?':
                        printf("Unknown option or ambiguous match\n");
                        ret = 1;
                        break;

                default:
                        ret = 1;
                        printf ("?? getopt returned character code 0%o ??\n", c);
                }
        }
        if(!no_archive && argc - optind < 1) {
                fprintf(stderr, "Please provide an archive name.\n");
                cdb_usage();
                ret = 1;
        }
        if((the_action == kFind_By_ID) && (argc - optind < 2)) {
                fprintf(stderr, "Please specify an id to find.\n");
                cdb_usage();
                ret = 1;
        }
        argc -= optind;
        argv += optind;
        set_char_option(kArchive_target, argv[0]);
        if(argc > 1)
                the_key = argv[1];
        return ret;
}



void cdb_usage()
{
        fprintf(stderr,
                "  Usage: cryptar-db -d[fsbac] -a[ila] archive <id>\n"
                "    --database (-d): (f)ilenames, (s)ummaries, (b)locks\n"
                "                     (a)rchives,  (c)onstants, (k)eys\n"
                "                     s(e)rver constants\n"
                "    --action (-a): (f)ind by id number, (l)ist all ids, (s)how all ids\n"
                "    --help (-h): this message\n"
                "\n"
                "    an archive-name is always required except for -h\n"
                "    an id is required for action f (find by ID)\n"
                );
}



static void find_by_id()
{
        void *db_object;
        
        db_open(the_database);

        if(the_database == kFilenames)
                db_object = the_fetch_func(the_key);
        else                
                db_object = the_fetch_func((void *)atoi(the_key));
        the_display_func(db_object);
        return;
}



void display_all()
{
        DB *dbp;
        DBC *curs;
        int ret;
        DBT key, data;
        IOSBuf *ios;
        char *id_string = NULL;
        void *db_obj;

        db_open(the_database);
        dbp = db_ptr(the_database);
        assert(dbp);
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));

        ios = ios_new();
        if((ret = dbp->cursor(dbp, NULL, &curs, 0))) {
                g_warning("db cursor error");
                dbp->err(dbp, ret, "DB->cursor");
                db_error_exit(dbp, ret);
                exit(1);
        }
        while((ret = curs->c_get(curs, &key, &data, DB_NEXT)) != DB_NOTFOUND) {
                if(ret) {
                        g_warning("db cursor (c_get) error");
                        dbp->err(dbp, ret, "DB->get");
                        db_error_exit(dbp, ret);
                        exit(1);
                }
                ios_set_buffer(ios, data.data, data.size);
                if(the_database == kFilenames)
                        id_string = g_strndup(key.data, key.size);
                if(the_action == kShow_All_IDs) {
                        if(the_database == kFilenames) {
                                printf("%s\n", id_string);
                        } else {
                                printf("%d\n", db_get_key_int(key));
                        }
                } else switch(the_database) {
                case kFilenames:
                        db_obj = db_filename_from_stream(ios, id_string);
                        db_filename_display(db_obj);
                        break;
                case kSummary:
                        db_obj = db_summary_from_stream(ios, db_get_key_int(key));
                        db_summary_display(db_obj);
                        break;
                case kBlocks:
                        db_obj = db_block_from_stream(ios, db_get_key_int(key));
                        db_block_display(db_obj);
                        break;
                case kArchive:
                        db_obj = db_archive_from_stream(ios, db_get_key_int(key));
                        db_archive_display(db_obj);
                        break;
                case kKeys:
                        /*db_obj = db_key_from_stream(ios, db_get_key_int(key));*/
                        /*db_key_display(db_obj);*/
                        break;
                default:
                        fprintf(stderr, "Unexpected case in display_all: %d\n",
                                the_database);
                }
        }
        if((ret = curs->c_close(curs))) {
                g_warning("db cursor (c_close) error");
                dbp->err(dbp, ret, "DB->get");
                db_error_exit(dbp, ret);
                exit(1);
        }
        ios_free(ios);
        return;
}



