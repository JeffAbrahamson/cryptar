/*
  Copyright (C) Jeff Abrahamson 2002
  Copyright (C) by Andrew Tridgell 1996, 2000
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

#ifndef __CRYPTAR_H__
#define __CRYPTAR_H__

/* For file_struct: */
#include <time.h>
#include <sys/types.h>

#if HAVE_OFF64_T
#define OFF_T off64_t
#define STRUCT_STAT struct stat64
#else
#define OFF_T off_t
#define STRUCT_STAT struct stat
#endif
#define INO64_T ino_t
#define DEV64_T dev_t

/* ??? ### */
struct file_struct {
	unsigned flags;
	time_t modtime;
	OFF_T length;
	mode_t mode;

	INO64_T inode;
	/** Device this file lives upon */
	DEV64_T dev;

	/** If this is a device node, the device number. */
	DEV64_T rdev;
	uid_t uid;
	gid_t gid;
	char *basename;
	char *dirname;
	char *basedir;
	char *link;
	char *sum;
};

void finish_transfer(char *fname, char *fnametmp, struct file_struct *file);

#endif
