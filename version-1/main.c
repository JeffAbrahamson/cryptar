/* -*- c-file-style: "linux" -*-

   Copyright (C) 2002-2004 by Jeff Abrahamson
   Copyright (C) 1996-2001 by Andrew Tridgell <tridge@samba.org>
   Copyright (C) Paul Mackerras 1996
   Copyright (C) 2001, 2002 by Martin Pool <mbp@samba.org>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <glib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* #include "cryptar.h" */
#include "config.h"

#include "archive.h"
#include "compress.h"
#include "constants.h"
#include "coprocess.h"
#include "daemon.h"
#include "db_misc.h"
#include "encryption.h"
#include "errcode.h"
#include "io.h"
#include "list.h"
#include "log.h"
#include "main.h"
#include "option.h"
#include "options.h"
#include "prefs.h"
#include "protocol.h"
#include "util.h"
#include "workticket.h"



#define UNUSED(x) x __attribute__((__unused__))

static int start_remote_and_run();
static RETSIGTYPE sig_quit_handler(int);
static RETSIGTYPE sigchld_handler(int);
static int do_server_init();
#ifdef MAINTAINER_MODE
static int do_self_test(const char *program);
#endif

static GMainLoop *main_event_loop = NULL;
/* main_loop_running means that it should be running more than that it
 * is. It's a way of breaking out of the callback loop that archive
 * lives in. */
static int main_loop_running = 0;



/**
 * Start a client for either type of remote connection.  Work out
 * whether the arguments request a remote shell or cryptard connection,
 * and call the appropriate connection function, then run_client.
 *
 * Calls do_cmd to start remote and then calls appropriate method for
 * talking to the remote.
 **/
static int start_remote_and_run()
{
	pid_t pid;
        char *cmd;

        cmd = pref_get_value(PREF_DAEMON_COMMAND);
	if (int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
		g_message("cmd=%s\n", cmd);
	pid = do_cmd(cmd);
        if(!pid) {
                g_error("Failed to connect to remote host with command '%s'", cmd);
                return -1;
        }
        g_free(cmd);

        if(int_option(kOption_ping))
                return do_ping();
	if(int_option(kOption_extract) || int_option(kOption_backup))
                return do_server_init();

        g_assert_not_reached();
        return 0;               /* not reached */
}



/* Begin talking to the server. Archive and extract take this path. */
static int do_server_init()
{
        ProtoHello ph;

        db_open_client_dbs();
        memset(&ph, 0, sizeof(ph));
        ph.archive_id = const_get_int(PREF_ARCHIVE_ID);
        ph.archive_pass = const_get_int(PREF_ARCHIVE_PASS);
        ph.create_archive = const_get_int(PREF_MUST_CREATE);
        ph.message = "Client version 0.0 here!";
        send_hello(&ph);

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Event loop starting.");
        run_main_loop();
        io_close_io();
        wt_check_for_orphan_work();
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Event loop terminated.");
        
        return 0;
}



static RETSIGTYPE sig_quit_handler(int UNUSED(val))
{
	db_error_exit();
        exit(2);
}

static RETSIGTYPE sigchld_handler(int UNUSED(val))
{
#ifdef WNOHANG
	while (waitpid(-1, NULL, WNOHANG) > 0) ;
#endif
}


/**
 * This routine catches signals and tries to send them to gdb.
 *
 * Because it's called from inside a signal handler it ought not to
 * use too many library routines.
 *
 * @todo Perhaps use "screen -X" instead/as well, to help people
 * debugging without easy access to X.  Perhaps use an environment
 * variable, or just call a script?
 *
 * @todo The /proc/ magic probably only works on Linux (and
 * Solaris?)  Can we be more portable?
 **/
#ifdef MAINTAINER_MODE
static RETSIGTYPE cryptar_panic_handler(int UNUSED(whatsig))
{
	char cmd_buf[300];
	int ret;
#if OLD_USE_XTERM_FOR_GDB
	sprintf(cmd_buf, 
		"xterm -display :0 -T Panic -n Panic "
		"-e gdb /proc/%d/exe %d", 
		getpid(), getpid());
#else
	sprintf(cmd_buf, 
		"xterm -T Panic -n Panic "
		"-e gdb /proc/%d/exe %d", 
		getpid(), getpid());
#endif
	/* Unless we failed to execute gdb, we allow the process to
	 * continue.  I'm not sure if that's right. */
	ret = system(cmd_buf);
	if (ret)
		_exit(ret);
}
#endif



static void setup_signals()
{
	signal(SIGUSR1, sig_quit_handler);
	signal(SIGUSR2, sig_quit_handler);
	signal(SIGCHLD, sigchld_handler);
#ifdef MAINTAINER_MODE
	signal(SIGSEGV, cryptar_panic_handler);
	signal(SIGFPE, cryptar_panic_handler);
	signal(SIGABRT, cryptar_panic_handler);
	signal(SIGBUS, cryptar_panic_handler);
#endif /* def MAINTAINER_MODE */

#define SIGNAL_CAST (RETSIGTYPE (*)())

	signal(SIGINT, sig_quit_handler);
	signal(SIGHUP, sig_quit_handler);
        signal(SIGTERM, sig_quit_handler);

	/* Ignore SIGPIPE; we consistently check error codes and will
	 * see the EPIPE. */
        /* ### Is this true??? ### */
	signal(SIGPIPE, SIG_IGN);

        return;
}



int main(int argc,char *argv[])
{       
	int ret;

        setenv("MALLOC_CHECK_", "2", 1); /* ### DEBUG!! ### */
	if (argc < 2) {
		usage();
                return 0;
	}
        /* The server will talk to the client on stdin/stdout, so all
         * logging, warnings, and errors must go to stderr or another
         * file descriptor. This makes the glib logging functions
         * always go to stderr.
         */
        g_log_set_handler (NULL,
                           G_LOG_LEVEL_DEBUG
                           | G_LOG_LEVEL_INFO
                           | G_LOG_LEVEL_MESSAGE
                           | G_LOG_LEVEL_CRITICAL
                           | G_LOG_LEVEL_ERROR
                           | G_LOG_LEVEL_WARNING
                           | G_LOG_FLAG_FATAL
                           | G_LOG_FLAG_RECURSION,
                           log_default_log_handler,
                           NULL);

        setup_signals();
        g_atexit(db_error_exit);
	if(parse_options(argc, argv)) {
		fprintf(stderr, "Option parsing error.\n");
                return 1;
	}

#ifdef CLEARTEXT
        /* The server doesn't do encryption or compression anyway, so
         * no need to warn if it was compiled in plaintext mode. */
        if(!int_option(kOption_server)) {
                g_warning("OPERATING IN CLEARTEXT MODE.");
                g_warning("This is a compile-time option for debugging.");
                g_warning("    The remote block store will contain unencrypted data!");
        }
#endif
        init_crypto();

	/* Initialize push_dir here because on some old systems getcwd
	   (implemented by forking "pwd" and reading its output)
	   doesn't work when there are other child processes.  Also,
	   on all systems that implement getcwd that way "pwd" can't
	   be found after chroot. */
        
        /* ### Do we need this? It surely doesn't hurt, but may not do
               anything. -JMA ### */
	push_dir(NULL,0);

#ifdef MAINTAINER_MODE
        if(int_option(kOption_self_test))
                return do_self_test(argv[0]);
#endif
        if(int_option(kOption_create)) {
                do_create();
                return 0;
        }
	if(int_option(kOption_server))
                return do_daemon();
        if(int_option(kOption_list)) {
                db_open_client_dbs();
                return do_list();
        }

	ret = start_remote_and_run();
        db_close_all();
        /* ### wait for child here ### */

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("About to exit from main");
	exit(0);
}




void run_main_loop()
{
        if(!main_event_loop) {
                main_event_loop = g_main_new(0);
                main_loop_running = 1;
        }
        g_main_run(main_event_loop);
}




/* This is for the benefit of client functions that iterate over files
 * and so don't want to lose themselves in a main event loop. The
 * issue is that both of the main event loop and the file iterator
 * (option archive) expect to be in control and use callback
 * functions. */
void cycle_main_loop()
{
        if(!main_event_loop)
                main_event_loop = g_main_new(0);
        g_main_iteration(0);
        g_main_iteration(0);
        g_main_iteration(0);
        return;
}



void quit_main_loop()
{
        main_loop_running = 0;
        g_main_quit(main_loop());
        return;
}



int is_main_loop_running()
{
        return main_loop_running;
}



GMainLoop *main_loop()
{
        return main_event_loop;
}



#ifdef MAINTAINER_MODE
static int do_self_test(const char *program)
{
        char *command;
	pid_t pid;
        
        g_assert(program);
        crypt_test();
        
        command = g_strconcat(program, " --daemon --verbose=127", NULL);
        g_message("command='%s'\n", command);
        pid = do_cmd(command);
        do_ping();
        return 0;
}
#endif

