/* 
   Copyright (C) Jeff Abrahamson 2002
   Copyright (C) Andrew Tridgell 1996
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

/* this file contains code used by more than one part of the rsync
   process */

#include <stdio.h>

#include "config.h"

#include "cryptar.h"


/* finish off a file transfer, renaming the file and setting the permissions
   and ownership */
void finish_transfer(char *fname, char *fnametmp, struct file_struct *file)
{
#if 0                           /* not yet */
	if (make_backups && !make_backup(fname))
		return;

	/* move tmp file over real file */
	if (robust_rename(fnametmp,fname) != 0) {
		if (errno == EXDEV) {
			/* rename failed on cross-filesystem link.  
			   Copy the file instead. */
			if (copy_file(fnametmp,fname, file->mode & INITACCESSPERMS)) {
				rprintf(FERROR,"copy %s -> %s : %s\n",
					fnametmp,fname,strerror(errno));
			} else {
				set_perms(fname,file,NULL,0);
			}
		} else {
			rprintf(FERROR,"rename %s -> %s : %s\n",
				fnametmp,fname,strerror(errno));
		}
		do_unlink(fnametmp);
	} else {
		set_perms(fname,file,NULL,0);
	}
#else
        fprintf(stderr, "Function finish_transfer is not yet defined.\n");
        fname = fnametmp;
        file = NULL;
#endif
}
