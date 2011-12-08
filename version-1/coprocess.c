/* -*- c-file-style: "linux" -*-

   Copyright (C) 2002, 2003 by Jeff Abrahamson
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


#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"

#include "cleanup.h"
#include "coprocess.h"
#include "errcode.h"
#include "option.h"
#include "options.h"
#include "io.h"


static void set_blocking(int fd);
static void set_nonblocking(int fd);
static int fd_pair(int fd[2]);
static pid_t piped_child(char **command, int *f_in, int *f_out);
static void print_child_argv(char **cmd);


/* work out what fcntl flag to use for non-blocking */
#ifdef O_NONBLOCK
# define NONBLOCK_FLAG O_NONBLOCK
#elif defined(SYSV)
# define NONBLOCK_FLAG O_NDELAY
#else 
# define NONBLOCK_FLAG FNDELAY
#endif


/****************************************************************************
Set a fd into nonblocking mode
****************************************************************************/
static void set_nonblocking(int fd)
{
	int val;

	if((val = fcntl(fd, F_GETFL, 0)) == -1)
		return;
	if (!(val & NONBLOCK_FLAG)) {
		val |= NONBLOCK_FLAG;
		fcntl(fd, F_SETFL, val);
	}
}



/****************************************************************************
Set a fd into blocking mode
****************************************************************************/
static void set_blocking(int fd)
{
	int val;

	if((val = fcntl(fd, F_GETFL, 0)) == -1)
		return;
	if (val & NONBLOCK_FLAG) {
		val &= ~NONBLOCK_FLAG;
		fcntl(fd, F_SETFL, val);
	}
}



/* create a file descriptor pair - like pipe() but use socketpair if
   possible (because of blocking issues on pipes)

   always set non-blocking
 */
static int fd_pair(int fd[2])
{
	int ret;

#if HAVE_SOCKETPAIR
	ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
#else
	ret = pipe(fd);
#endif

	if (ret == 0) {
		set_nonblocking(fd[0]);
		set_nonblocking(fd[1]);
	}
	
	return ret;
}



static void print_child_argv(char **cmd)
{
        char *cmd_string;

        g_assert(cmd);
        cmd_string = g_strjoinv(" ", cmd);
        g_message("opening connection to server: %s", cmd_string);
#if 0
	for (; *cmd; cmd++) {
		/* Look for characters that ought to be quoted.  This
		* is not a great quoting algorithm, but it's
		* sufficient for a log message. */
		if (strspn(*cmd, "abcdefghijklmnopqrstuvwxyz"
			   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			   "0123456789"
			   ",.-_=+@/") != strlen(*cmd)) {
			fprintf(stderr, "\"%s\" ", *cmd);
		} else {
			fprintf(stderr, "%s ", *cmd);
		}
	}
	fprintf(stderr, "\n");
#endif
}



/* Note that in the child STDIN is set to blocking and STDOUT is set
   to non-blocking. This is necessary as rsh relies on stdin being
   blocking and ssh relies on stdout being non-blocking. (But people
   should always use ssh anyway.)

   If blocking_io is set then use blocking io on both fds. That can be
   used to cope with badly broken rsh implementations like the one on
   solaris.

   These comments are from rsync, are they true in our context? We
   probably always talk over a local pipe and let the process we talk
   to (ssh, whatever) deal with the network.
 */
static pid_t piped_child(char **command, int *f_in, int *f_out)
{
	pid_t pid;
	int to_child_pipe[2];
	int from_child_pipe[2];
        
	if (int_option(kOption_verbose) & VERBOSE_FLOW) {
		print_child_argv(command);
	}

	if (fd_pair(to_child_pipe) < 0 || fd_pair(from_child_pipe) < 0) {
		g_warning("pipe: %s\n", strerror(errno));
		g_assert_not_reached();
	}

	pid = fork();
	if (pid < 0) {
		g_warning("fork: %s\n", strerror(errno));
		g_assert_not_reached();
	}

	if (pid == 0) {
                /* child */
                close(to_child_pipe[1]);
                close(from_child_pipe[0]);
                
		if(to_child_pipe[0] != STDIN_FILENO) {
                        if(dup2(to_child_pipe[0], STDIN_FILENO) < 0) {
                                g_warning("Failed to dup/close : %s\n",
                                        strerror(errno));
                                g_assert_not_reached();
                        }
                        close(to_child_pipe[0]);
                }
		if(from_child_pipe[1] != STDOUT_FILENO) {
                        if(dup2(from_child_pipe[1], STDOUT_FILENO) < 0) {
                                g_warning("Failed to dup/close : %s\n",
                                        strerror(errno));
                                g_assert_not_reached();
                        }
                        close(from_child_pipe[1]);
                }
			
		set_blocking(STDIN_FILENO);
                set_blocking(STDOUT_FILENO);
		execvp(command[0], command);
		g_warning("Failed to exec %s : %s\n",
			command[0], strerror(errno));
		g_assert_not_reached();
	}

        /* parent */
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("server local pid = %d", pid);
	if (close(from_child_pipe[1]) < 0 || close(to_child_pipe[0]) < 0) {
		g_warning("Failed to close : %s\n", strerror(errno));
		g_assert_not_reached();
	}

	*f_out = to_child_pipe[1];
	*f_in = from_child_pipe[0];
        set_nonblocking(*f_out);
        set_nonblocking(*f_in);

	return pid;
}



/* Start the remote process (server) with bi-directional pipe. */
pid_t do_cmd(const char *cmd)
{
	char *args[100];
	int i,argc=0;
	pid_t ret;
	char *tok,*dir=NULL;
        int f_in, f_out;
        
        if (!cmd) {
                g_warning("No command in do_cmd.  Sorry, can't recover.");
                return 0;
        }
        cmd = g_strdup(cmd);
        
        for (tok=strtok((char *)cmd," "); tok; tok=strtok(NULL," ")) {
                args[argc++] = tok;
        }
	args[argc] = NULL;

	if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS) {
		fprintf(stderr,"cmd=");
		for (i=0;i<argc;i++)
			fprintf(stderr,"%s ",args[i]);
		fprintf(stderr,"\n");
	}

        ret = piped_child(args, &f_in, &f_out);
        io_register_fd_pair(f_in, f_out);
        
	if (dir) free(dir);

	return ret;
}

