/*  daemon.c
 *  Copyright (C) 2002 by:
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
#include <unistd.h>

#include "config.h"

#include "daemon.h"
#include "io.h"
#include "main.h"
#include "option.h"
#include "options.h"
#include "remote.h"


#define MAX_BUF 1000


/*
  Real do_daemon:

  - REP loop
  - commands:
  - - store block
  - - retrieve block
  - - delete block

 */

int do_daemon()
{
        if(!confirm_arguments(0, 0))
                return -1;

        io_register_fd_pair(STDIN_FILENO, STDOUT_FILENO);
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Daemon starting main loop.");
        run_main_loop();
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Daemon finished main loop.");
        io_close_io();
        return 0;
}

