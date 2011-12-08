/*  encryption.h
 *  Copyright (C) 2002, 2003 by:
 *  Jeff Abrahamson, Adam O'Donnell
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


#ifndef __CRYPT_H__
#define __CRYPT_H__

#define DEBUG_CRYPTO 1
#define RANDOM_KEYSIZE 512

#include <glib.h>


/* Forward declarations */
struct ios_buf;
typedef unsigned char *Key;              /* This should probably be allocated to something? */


void init_crypto(void);
void print_key(void);
void string_to_key(Key key);
void set_key(char *inputkey, int len); 
void generate_random_key(void);
struct ios_buf *encrypt(struct ios_buf *ios);
struct ios_buf *decrypt(struct ios_buf *ios);
gint block_size(gint block_size);


#endif
