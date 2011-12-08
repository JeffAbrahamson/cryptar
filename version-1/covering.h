/*  covering.h
 *  Copyright (C) 2002-2004 by:
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


#ifndef __COVERING_H__
#define __COVERING_H__

#include <glib.h>
#include <stdio.h>

#include "hash.h"

struct db_block;
struct signature;


GPtrArray *cover_new();
void cover_update_remote(FILE *fp, struct signature *sig);
void cover_fill_block(struct db_block *dbb, FILE *fp);
void cover_save_local(GPtrArray *block_list);

#endif
