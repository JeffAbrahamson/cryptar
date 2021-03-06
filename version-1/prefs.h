/*  prefs.h
 *  Copyright (C) 2002-2004 by:
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
*/


#ifndef __PREFS_H__
#define __PREFS_H__

#include <glib.h>


#define CONFIG_DIR_VARNAME "CRYPTAR_CONFIG_DIR"
#define CONFIG_DIR "cryptar"

/* ### Why are these not an enum? ### */
/* Valid preference key values */
#define PREF_BLOCK_LENGTH "block_length"
#define PREF_DAEMON_COMMAND "daemon_command"
#define PREF_TO_ARCHIVE "to_archive"
#define PREF_ALWAYS_COMPARE_HASH "always_compare_hash"
#define PREF_ARCHIVE_ID "archive_id"
#define PREF_ARCHIVE_PASS "archive_pass"
#define PREF_MUST_CREATE "must_create"

/* Some default values */
#define DEFAULT_BLOCK_LENGTH "1024"

void do_create();
char *pref_get_value(char *key);
int pref_get_int_value(char *key);
char *pref_get_value_silent_default(char *key, const char *deflt);
int pref_get_int_value_silent_default(char *key, const char *deflt);

void pref_get_prefs();

#endif
