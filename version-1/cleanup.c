/* -*- c-file-style: "linux" -*-
   
   Copyright (C) 1996-2000 by Andrew Tridgell
   Copyright (C) Paul Mackerras 1996
   
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

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"

#include "cleanup.h"
#include "errcode.h"
#include "option.h"
#include "options.h"



/* handling the cleanup when a transfer is interrupted is tricky when
   --partial is selected. We need to ensure that the partial file is
   kept if any real data has been transferred */
int cleanup_got_literal=0;
static int cleanup_pid = 0;

pid_t cleanup_child_pid = -1;



/*
 * Code is one of the RERR_* codes from errcode.h.
 */
void _exit_cleanup(int code, const char *file, int line)
{
	int ocode = code;
        
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);

	if (int_option(kOption_verbose) & VERBOSE_FLOW)
		g_message("_exit_cleanup(code=%d, file=%s, line=%d): entered\n", 
                          code, file, line);

	if (cleanup_child_pid != -1) {
		int status;
		if (waitpid(cleanup_child_pid, &status, WNOHANG) == cleanup_child_pid) {
			status = WEXITSTATUS(status);
			if (status > code) code = status;
		}
	}

	if (int_option(kOption_verbose) & VERBOSE_FLOW)
		fprintf(stderr,"_exit_cleanup(code=%d, file=%s, line=%d): about to call exit(%d)\n", 
			ocode, file, line, code);

        fprintf(stderr, "Exiting from _exit_cleanup with code %d.\n", code);
	exit(code);
}
