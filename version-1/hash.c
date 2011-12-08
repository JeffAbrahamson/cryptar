/*  hash.c
 *  Copyright (C) 2002, 2003 by:
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


#include <assert.h>
#include <glib.h>
#include <search.h>

#include "hash.h"
#include "options.h"


/* We are going to hash 32 bit integers that we believe to be evenly
   distributed. So our hash function can be quite simple, just mask
   with 0xFFFF. Just in case, though, we offer a switch to gather
   statistics on how well we're doing.

   We maintain a single static hash table.  If we intend to use
   multiple hash tables at the same time, we'd need to pass the hash
   table pointer to and from these calls.
*/

static void hash_clear();
static int hash_find_comparator(gconstpointer arg_item, gconstpointer arg_key);
#if EXPENSIVE_CHECK
static void hash_verify();
static void hash_verify_entry(gpointer entry, gpointer user_data);
#endif
     

/* If non-zero, gather statistics */
#define GATHER_HASH_STATISTICS 1

typedef GSList *HashBucket;

typedef struct hashentry {
        guint32 key;
        guint32 value;
} HashEntry;

/* Since we don't grow our hash tables, a hash table is simply an
   array of buckets. */
static HashBucket *hash_table = NULL;
static GMemChunk *entry_pool = NULL;
static const guint32 hash_size = (1 << 16);

#if GATHER_HASH_STATISTICS
static guint32 hash_entries;
static guint32 hash_collisions;
#endif



/* Make a new hash table.  If one already exists, free it and
   reinitialize it to be empty.  Note that a null pointer for a bucket
   means an empty bucket, we'll initialize the bucket only if we have
   something to put in it.
 */
void hash_create()
{
        if(!entry_pool)
                entry_pool = g_mem_chunk_create(HashEntry, 1024, G_ALLOC_ONLY);
        g_assert(entry_pool);
        if(hash_table) {
                hash_destroy();
        }
        hash_table = g_new0(HashBucket, hash_size);
#if GATHER_HASH_STATISTICS
        hash_entries = 0;
        hash_collisions = 0;
#endif
        return;
}



/* Deallocate all buckets.
 */
static void hash_clear()
{
        guint32 i;

        g_assert(hash_table);
        for(i = 0; i < hash_size; i++) {
                if(hash_table[i])
                        g_slist_free(hash_table[i]);
        }
        g_assert(entry_pool);
        g_mem_chunk_destroy(entry_pool);
        entry_pool = 0;
        return;
}



/* Return non-zero and set *value if key is in hash table.
   On zero return, *value is undefined.
 */
int hash_find(guint32 key, guint32 *value)
{
        guint16 hash_index;
        HashBucket this_bucket;
        GSList *list_item;
        HashEntry *entry;

        g_assert(value);
        hash_index = key & 0xffff;
        this_bucket = hash_table[hash_index];
        if(!this_bucket) return 0;
        
        list_item = g_slist_find_custom(this_bucket,
                                        GINT_TO_POINTER((guint32)hash_index),
                                        hash_find_comparator);
        if(list_item) {
                entry = list_item->data;
                g_assert(entry);
                *value = entry->value;
                return 1;
        }
        return 0;
}



static int hash_find_comparator(gconstpointer arg_item, gconstpointer arg_key)
{
        guint32 key;
        HashEntry *entry;
        
        g_assert(arg_item);
        entry = (HashEntry *)arg_item;
        g_assert(entry);
        key = GPOINTER_TO_INT(arg_key);
        if(entry->key == key)
                return 0;       /* found */
        return 1;
}



/* Free a hash table and its contents.
 */
void hash_destroy()
{
        if(!hash_table)
                return;
        
#if GATHER_HASH_STATISTICS
        g_message("Hash table statistics: %d entries, of which %d collisions.",
                  hash_entries, hash_collisions);
#endif
        hash_clear();
        g_free(hash_table);
        hash_table = 0;
        return;
}



/* Insert value into the hash table in the bucket specified by key.
 */
void hash_insert(guint32 key, guint32 value)
{
        guint16 hash_index;
        HashBucket this_bucket;
        HashEntry *entry;

        hash_index = key & 0xffff;
        this_bucket = hash_table[hash_index];
#if GATHER_HASH_STATISTICS
        hash_entries++;
        if(this_bucket)
                hash_collisions++;
        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                g_message("[WCsum] (%d,%d) 0x%x %d",
                          hash_entries, hash_collisions, hash_index, value);
#endif
        entry = g_chunk_new(HashEntry, entry_pool);
        g_assert(entry);
        entry->key = hash_index;
        entry->value = value;
        this_bucket = g_slist_append(this_bucket, entry);
        hash_table[hash_index] = this_bucket;
        return;
}



#if EXPENSIVE_CHECK
/* Check the entire hash table.  Verify that any allocated buckets
   contain data that is at least addressable and reasonable.  We can't
   do much, but we might be able to segfault if the data is bad.
 */
static void hash_verify()
{
        guint32 i;

        g_assert(hash_table);
        for(i = 0; i < hash_size; i++) {
                if(hash_table[i]) {
                        g_assert(hash_table[i]->data);
                        g_slist_foreach(hash_table[i], hash_verify_entry, 0);
                }
        }
        g_assert(entry_pool);
        return;
}



static void hash_verify_entry(gpointer entry, gpointer user_data)
{
        HashEntry *he;
        guint32 x,y;
        
        user_data = 0;          /* value ignored */
        g_assert(entry);
        he = (HashEntry *)entry;
        x = he->key;  /* just confirm that addreses are addressable */
        y = he->value;
        return;
}
#endif


