/* fileio.h
   Copyright (C) Jeff Abrahamson 2002
   Copyright (C) Andrew Tridgell 1998
   Copyright (C) 2002 by Martin Pool
   
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

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <sys/types.h>

#if HAVE_OFF64_T
#define OFF_T off64_t
#define STRUCT_STAT struct stat64
#else
#define OFF_T off_t
#define STRUCT_STAT struct stat
#endif

int sparse_end(int f);
int write_file(int f,char *buf,size_t len);
struct map_struct *map_file(int fd,OFF_T len);
char *map_ptr(struct map_struct *map,OFF_T offset,int len);
void unmap_file(struct map_struct *map);

#endif
