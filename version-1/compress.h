/*  compress.h
 *  Copyright (C) 2002-2003 by:
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


#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include <glib.h>


/* Forward declarations */
struct proto_put_block;
struct proto_get_block_ack;

struct ios_buf *do_compress(struct ios_buf *ios);
struct ios_buf *do_decompress(struct ios_buf *ios, guint32 orig_len);

void crypt_test();

#endif
