/*  prefs.c
 *  Copyright (C) 2004 by:
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


#include <glib.h>
#include <string.h>

#include "constants.h"
#include "db_misc.h"
#include "ios.h"
#include "options.h"



void const_put_string(const char *key, const char *val)
{
        IOSBuf *ios_key, *ios_data;

        g_assert(key);
        g_assert(val);
        ios_key = ios_new();
        ios_data = ios_new();

        ios_set_buffer(ios_key, key, strlen(key));
        ios_append_string(ios_data, val);
        db_put(kConstants, ios_key, ios_data);
        ios_free(ios_key);
        ios_free(ios_data);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("constant: %s <-- %s", key, val);
        return;
}



/*
  Returned value must be freed by caller.
  Returns NULL if key is not found in database.
*/
char *const_get_string(const char *key)
{
        IOSBuf *ios_key, *ios_data;
        char *ret_buf, *val;
        
        g_assert(key);
        ios_key = ios_new();
        ios_data = ios_new();

        ios_set_buffer(ios_key, key, strlen(key));
        db_get(kConstants, ios_key, ios_data);
        val = ios_read_str(ios_data);
        if(val)
                ret_buf = g_strdup(val);
        else
                ret_buf = NULL;
        ios_free(ios_key);
        ios_free(ios_data);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("constant: %s --> %s", key, ret_buf);
        return ret_buf;
}



void const_put_int(const char *key, const guint32 val)
{
        IOSBuf *ios_key, *ios_data;

        g_assert(key);
        ios_key = ios_new();
        ios_data = ios_new();

        ios_set_buffer(ios_key, key, strlen(key));
        ios_append_int32(ios_data, val);
        db_put(kConstants, ios_key, ios_data);
        ios_free(ios_key);
        ios_free(ios_data);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("constant: %s <-- %d", key, val);
        return;
}



int const_get_int(const char *key)
{
        IOSBuf *ios_key, *ios_data;
        guint32 val;
        
        g_assert(key);
        ios_key = ios_new();
        ios_data = ios_new();

        ios_set_buffer(ios_key, key, strlen(key));
        db_get(kConstants, ios_key, ios_data);
        val = ios_read_int32(ios_data); /* ### Hmm, what happens if we don't have one? */
        ios_free(ios_key);
        ios_free(ios_data);
        if(int_option(kOption_verbose) & VERBOSE_DB)
                g_message("constant: %s --> %d", key, val);
        return val;
}



