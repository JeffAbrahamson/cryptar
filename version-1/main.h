/* -*- c-file-style: "linux" -*-

   Copyright (C) 2002 by Jeff Abrahamson
   
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


#ifndef __MAIN_H__
#define __MAIN_H__

#include <glib.h>
#include <sys/types.h>


void run_main_loop();
void cycle_main_loop();
GMainLoop *main_loop();
int main(int argc,char *argv[]);
void quit_main_loop();
int is_main_loop_running();

#endif
