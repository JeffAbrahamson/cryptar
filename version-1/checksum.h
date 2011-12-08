/* checksum.h
   Copyright (C) Jeff Abrahamson 2002-2003
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


#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

#include <glib.h>
#include <openssl/sha.h>
#include <stdio.h>

#include "hash.h"

struct db_block;


#define SCsumLength SHA_DIGEST_LENGTH
typedef guchar SCsum[SCsumLength];
typedef guchar *SCsumPtr;

guint32 WCsum_get(signed char *buf, guint16 len);
guint32 WCsum_update(signed char *old4, signed char *new4, guint32 old_sum, guint32 len);
void WCsum_make_hash(FILE *fp, guint16 block_len);

SCsumPtr SCsum_buffer(const unsigned char *buf, guint16 len, SCsumPtr sc);
void SCsum_file(FILE *fp, SCsumPtr sc);
int SCsum_match(FILE *fp, guint32 offset, struct db_block *block);
int SCsum_cmp(SCsumPtr sc1, SCsumPtr sc2);
void SCsum_print(SCsumPtr sc);

#endif
