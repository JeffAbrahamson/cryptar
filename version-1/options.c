/*  options.c
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


#include <assert.h>
#include <string.h>

#include "options.h"


GPtrArray *g_args = NULL;


/* Should this perhaps use the same tree structure that prefs.c does?
   Maybe they should even be the same interface.  Is there really a
   difference between character options and preferences?
*/
static int int_option_array[kNum_int_options];
static char *char_option_array[kNum_char_options];


void options_init()
{
        memset(int_option_array, sizeof(int_option_array), 0);
        memset(char_option_array, sizeof(char_option_array), 0);
        return;
}



void set_int_option(enum int_options opt, int val)
{
        assert(opt < kNum_int_options);
        int_option_array[opt] = val;
        return;
}



int int_option(enum int_options opt)
{
        assert(opt < kNum_int_options);
        return int_option_array[opt];
}



void set_char_option(enum char_options opt, char *val)
{
        assert(opt < kNum_char_options);
        char_option_array[opt] = val;
        return;
}



char *char_option(enum char_options opt)
{
        assert(opt < kNum_char_options);
        return char_option_array[opt];
}



char *char_option_safe(enum char_options opt)
{
        assert(opt < kNum_char_options);
        if(char_option_array[opt])
                return char_option_array[opt];
        return "";
}



