/* syscall.h
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

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#if HAVE_OFF64_T
#define OFF_T off64_t
#define STRUCT_STAT struct stat64
#else
#define OFF_T off_t
#define STRUCT_STAT struct stat
#endif

int do_unlink(char *fname);
int do_symlink(char *fname1, char *fname2);
int do_lchown(const char *path, uid_t owner, gid_t group);
int do_mknod(char *pathname, mode_t mode, dev_t dev);
int do_rmdir(char *pathname);
int do_open(char *pathname, int flags, mode_t mode);
int do_chmod(const char *path, mode_t mode);
int do_rename(char *fname1, char *fname2);
void trim_trailing_slashes(char *name);
int do_mkdir(char *fname, mode_t mode);
int do_mkstemp(char *template, mode_t perms);
int do_stat(const char *fname, STRUCT_STAT *st);
int do_lstat(const char *fname, STRUCT_STAT *st);
int do_fstat(int fd, STRUCT_STAT *st);
OFF_T do_lseek(int fd, OFF_T offset, int whence);
void *do_mmap(void *start, int len, int prot, int flags, int fd, OFF_T offset);

#endif
