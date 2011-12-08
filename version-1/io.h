/*  io.h
 *  Copyright (C) 2002-2003 by:
 *  Jeff Abrahamson
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
*/


#ifndef __IO_H__
#define __IO_H__


struct ios_buf;
struct _GIOChannel;

void io_register_fd_pair(int fd_in, int fd_out);
struct ios_buf *io_write_buf();
void io_write_reg();
struct ios_buf *io_read_buf();
void io_send_ios(struct ios_buf *ios);
void io_no_more_input();
void io_no_more_output();
void io_close_io();

#endif

