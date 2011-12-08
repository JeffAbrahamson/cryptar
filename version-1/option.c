/*  option.c
 *  Copyright (C) 2002-2004 by:
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


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <glib.h>

#include "config.h"

#include "log.h"
#include "option.h"
#include "options.h"


enum {
        kDaemon = 1000,
#ifdef MAINTAINER_MODE
        kSelfTest,
#endif
        kLast                   /* Not needed except to avoid comma fault */
};


static void show_flags();
static int flags_consistent();


static struct option long_options[] = {
        {"backup", no_argument, NULL, 'b'},
        {"create", no_argument, NULL, 'c'},
        {"extract", no_argument, NULL, 'x'},
        {"list", no_argument, NULL, 't'},
        {"ping", no_argument, NULL, 'p'},
        {"verbose", optional_argument, NULL, 'v'},
        {"dry-run", no_argument, NULL, 'n'},
        {"daemon", no_argument, NULL, kDaemon},
        {"help", no_argument, NULL, 'h'},
#ifdef MAINTAINER_MODE
        {"selftest", no_argument, NULL, kSelfTest},
#endif
        {0, 0, 0, 0}
};


static int num_args = -1;            /* Number of arguments to process
                                   after option processing. */


int parse_options(int argc, char **argv)
{

        int c;

        options_init();
        while (c = getopt_long (argc, argv, "bcxtpv::nh", long_options, 0),
               c != EOF) {

                switch (c) {

                case 'b':
                        log_am_client();
                        set_int_option(kOption_backup, 1);
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message ("action backup");
                        break;
                        
                case 'c':
                        log_am_client();
                        set_int_option(kOption_create, 1);
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message ("action create");
                        break;

                case 'x':
                        log_am_client();
                        set_int_option(kOption_extract, 1);
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message("action extract");
                        break;
                        
                case 't':
                        log_am_client();
                        set_int_option(kOption_list, 1);
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message("option list");
                        break;

                case 'p':
                        log_am_client();
                        set_int_option(kOption_ping, 1);
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message("option ping");
                        break;
                        
                case 'v':
                        if(optarg && atoi(optarg)) {
                                set_int_option(kOption_verbose, atoi(optarg));
                        } else if(optarg) {
                                g_warning("Bad argument to --verbose: %s",
                                        optarg);
                                return 1;
                        } else {
                                set_int_option(kOption_verbose, VERBOSE_FILES);
                        }
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message ("option verbose = %d",
                                           int_option(kOption_verbose));
                        break;

                case 'n':
                        set_int_option(kOption_dryrun, 1);
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message("option dry-run");
                        set_int_option(kOption_verbose,
                                       int_option(kOption_verbose | VERBOSE_FILES));
                        g_error("This option isn't currently implemented.");
                        break;

                case kDaemon:
                        log_am_server();
                        set_int_option(kOption_server, 1);
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message ("server mode");
                        break;

                case 'h':
                        usage();
                        exit(0);
                        break;

#ifdef MAINTAINER_MODE
                case kSelfTest:
                        log_am_client();
                        set_int_option(kOption_self_test, 1);
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message("self test");
                        break;
#endif

                case ':':
                        g_warning("Required argument missing.");
                        return 1;
                        break;
                        
                case '?':
                        g_warning("Unknown option or ambiguous match");
                        return 1;
                        break;

                default:
                        g_warning ("?? getopt returned character code 0%o ??", c);
                        return 1;
                }
        }

        num_args = argc - optind;
        if(optind < argc) {
                set_char_option(kArchive_target, argv[optind++]);
                if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                        g_message ("Archive is '%s'",
                                   char_option_safe(kArchive_target));
        }
        g_args = g_ptr_array_new();
        while(optind < argc)
                g_ptr_array_add(g_args, argv[optind++]);

        if(!(int_option(kOption_backup) || int_option(kOption_extract)
#ifdef MAINTAINER_MODE
             || int_option(kOption_self_test)
#endif
             || int_option(kOption_list) || int_option(kOption_server)
             || int_option(kOption_ping) || int_option(kOption_create))) {
                g_message("You must choose an action to perform.");
                return 1;
        }
        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS) {
                g_message("Arguments:");
                while(optind < argc)
                        g_message("\t%s", argv[optind++]);
        }

        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS) show_flags();
        if(flags_consistent())
                return(0);      /* All ok */
        return(1);              /* error */
}



static void show_flags()
{
        g_message("archive_target = %s", char_option(kArchive_target));
        g_message("action_backup = %d ('%s')", int_option(kOption_backup),
                  char_option_safe(kBackup_pattern));
        g_message("action_extract = %d ('%s')", int_option(kOption_extract),
                  char_option_safe(kExtract_pattern));
        g_message("action_list = %d ('%s')", int_option(kOption_list),
                  char_option_safe(kList_pattern));
        g_message("action_ping = %d", int_option(kOption_ping));
        g_message("action_server = %d", int_option(kOption_server));
        
        g_message("option_verbose = %d", int_option(kOption_verbose));
        g_message("option_dryrun = %d", int_option(kOption_dryrun));
        
        return;
}



static int flags_consistent()
{
        if(int_option(kOption_backup) + int_option(kOption_extract) +
           int_option(kOption_list) + int_option(kOption_server) > 1) {
                g_message("Too many actions specified.");
                return(0);
        }
        if(int_option(kOption_backup) + int_option(kOption_extract) +
           int_option(kOption_list) + int_option(kOption_ping) > 1) {
                g_message("Too many actions specified.");
                return(0);
        }
        if((int_option(kOption_backup) | int_option(kOption_extract) |
            int_option(kOption_list) | int_option(kOption_server) |
            int_option(kOption_ping)) & !char_option(kArchive_target)) {
                g_message("Must specify a database.");
                return(0);
        }
        return(1);
}



/* Confirm that we have precisely n arguments to process.
   If more is non-zero, then accept at least n arguments.
 */
int confirm_arguments(int n, int more)
{
        if(num_args == n)
                return TRUE;
        if(num_args > n && more) {
                return TRUE;
        }

        g_message("Expecting %d arguments, found %d.", n, num_args);
        return FALSE;
}



void usage()
{
        fprintf(stderr, "  Cryptar, Copyright (C) 2004 Jeff Abrahamson\n  Cryptar comes with ABSOLUTELY NO WARRANTY.\n  This is free software, and you are welcome to redistribute it\n  under certain conditions.\n  See the file COPYING that came with this distribution of cryptar,\n  or see http://www.fsf.org/licenses/gpl.txt.\n\n");
        
        fprintf(stderr, "Usage/version info should be here.\n");

        fprintf(stderr, "\t--archive, -a:  archive files\n");
        fprintf(stderr, "\t--create, -c:   create a new archive\n");
        fprintf(stderr, "\t--extract, -x:  extract files from an archive\n");
        fprintf(stderr, "\t--list, -t:     list files in an archive\n");
        fprintf(stderr, "\t--ping, -p:     ping a remote server\n");
        fprintf(stderr, "\t--verbose, -v:  be more verbose\n");
        fprintf(stderr, "\t     %d=files, %d=protocol, %d=IO, %d=IO summary\n",
                VERBOSE_FILES, VERBOSE_PROTOCOL, VERBOSE_IO, VERBOSE_IO_SUM);
        fprintf(stderr, "\t     %d=flow, %d=detail flow, %d=DB\n",
                VERBOSE_FLOW, VERBOSE_FLOW_PLUS, VERBOSE_DB);
        fprintf(stderr, "\t--dry-run, -n:  don't actually do anything\n");
        fprintf(stderr, "\t--daemon:       run as a server (block store)\n");
        fprintf(stderr, "\t--help, -h:     display this message\n");
#ifdef MAINTAINER_MODE
        fprintf(stderr, "\t--selftest:     test the protocol and some internal functions\n");
#endif
        fprintf(stderr, "\n");
        return;
}


     
