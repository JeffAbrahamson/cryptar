/*  sum.c
 *  Copyright (C) 2002 by:
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/



/* Conceptually, divide the file into non-overlapping adjacent blocks
   of length offset. Compute the weak and strong checksums for each
   block.

   The hash tables are keyed on the checksum with the (zero-based)
   block number as value. So the block number * offset gives the
   actual byte offset from the beginning of the file for the start of
   the block with that checksum.
 */

#if 0                           /* I think this is obsolete */

#include <glib.h>

#include "config.h"

#include "sum.h"
#include "hash.h"



typedef struct sparse_sums {
        guint32 offset;
        HashTable *weak;
        HashTable *strong;
        /*
        guint32 *weak;
        guint32 *strong;
        */
} SparseSums;

typedef struct dense_sum {
        int not_implemented_yet;
} DenseSum;



SparseSums *sparse_sums_from_file(const char *filename)
{
        SparseSums *s = 0;
        
        filename = 0;
        return s;
}



SparseSums *sparse_sums_from_db(const char *filename)
{
        SparseSums *s = 0;
        
        filename = 0;
        return s;
}



DenseSum *init_dense_sum(const char *filename)
{
        DenseSum *ds = 0;
        
        filename = 0;
        return ds;
}



int next_dense_sum(DenseSum *ds)
{
        ds = 0;
        return 0;
}



int jump_dense_sum(DenseSum *ds, int jump)
{
        ds = 0;
        jump = 0;
        return 0;
}



guint32 weak_checksum(char *data, guint32 count)
{
        data = 0;
        count = 0;
        return 0;
}




guint32 strong_checksum(char *data, guint32 count)
{
        data = 0;
        count = 0;
        return 0;
}


#endif

