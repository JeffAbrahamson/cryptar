/*  options-def.h
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



/* The idea is that this is included by options.c and options.h.
   They each define the macros for their own purposes: the .c to
   generate code, the .h to generate headers.
 */


int_option(action_archive);
char_option(archive_target);
int_option(action_extract);
int_option(action_list);
int_option(action_daemon);
int_option(action_ping);

int_option(option_verbose);
int_option(option_dryrun);
int_option(read_only);
int_option(list_only);

