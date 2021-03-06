/*  prefs.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "db_misc.h"
#include "option.h"
#include "options.h"
#include "prefs.h"
#include "syscall.h"


#ifndef NULL
#define NULL 0
#endif


#if 0
/* Do this instead with mmap. */
typedef struct file_contents {
        char *buf;
        guint len;
} FileContents;

typedef struct preference {
        char *name;
        char *value;
} Preference;

GTree *prefs = NULL;
#endif


static char *get_pref_from_user(const char *name, const char *description, const char *default_value);
#if 0
static gint PrefCompare(gconstpointer a, gconstpointer b);
static FileContents pref_read_pref_file(const char *file);
static void pref_parse_prefs(FileContents cont);
#endif



#define YESNO_SIZE 10
/*
  Interact with the user to create a new archive.
*/
void do_create()
{
        /* These char pointers will be malloc'ed for us, so we must
           free them at the end.
        */
        char *daemon_command, *to_archive, *block_length;
        int n_block_length;
        char buf[YESNO_SIZE];

        /* ### Good target for I18N ### */
        printf("\nThis will lead you through the steps to create a new archive.\n");

        daemon_command = get_pref_from_user("daemon command", "This is the local command to use to talk to the remote cryptar server.\nTypically it will be \"ssh hostname cryptar --daemon\", where you replace hostname\nwith the name of the host with which you're communicating.\n", "ssh host cryptar --daemon");
        to_archive = get_pref_from_user("files to archive", "This is a comma-separated list of files to archive.", getenv("HOME"));
        block_length = get_pref_from_user("block length", "This is the block size for use in the differencing algorithm.\nUnless you know what you are doing, leave this at the default value.",
#if MAINTAINER_MODE
                                          "4"
#else
                                          "512"
#endif
                                          );
        n_block_length = atoi(block_length);

        printf("\n\n");
        printf("daemon command = \"%s\"\n", daemon_command);
        printf("to archive = \"%s\"\n", to_archive);
        printf("block length = %d\n", n_block_length);
        printf("\nOK? [Y]es/no ");
        fgets(buf, YESNO_SIZE, stdin);
        if(buf[0] != 'y' && buf[0] != 'Y') {
                /* We could be more polite, we could offer to edit, etc. */
                printf("Sorry, try again when you know what you want.\n");
                return;
        }

        db_open(kConstants);
        const_put_string(PREF_DAEMON_COMMAND, daemon_command);
        const_put_string(PREF_TO_ARCHIVE, to_archive);
        const_put_int(PREF_BLOCK_LENGTH, n_block_length);
        /* ### srand(time()); ### */ const_put_int(PREF_ARCHIVE_PASS, rand());
        const_put_int(PREF_MUST_CREATE, 1); /* set to 0 after created */
        
        g_free(daemon_command);
        g_free(to_archive);
        g_free(block_length);

        printf("Local archive created.  The first time you connect to the server,\n");
        printf("the remote store will be created.  You can change the settings here\n");
        printf("with the --edit option.  (This isn't yet implemented...)\n");
        return;
}



#define BUFSIZE 10240
static char *get_pref_from_user(const char *name, const char *description, const char *default_val)
{
        char buf[BUFSIZE];

        g_assert(name);
        printf("\n\n");
        if(description)
                printf("%s\n", description);
        printf("%s [%s]: ", name, default_val);
        fgets(buf, BUFSIZE, stdin);
        g_strchomp(buf);
        if(buf[0] == '\0')
                return g_strdup(default_val);
        return g_strdup(buf);
}



/* Fetch a preference value, fail if it's not available. */
char *pref_get_value(char *key)
{
        char *value;

        g_assert(key);
        value = const_get_string(key);
        if(!value) {
                g_warning("Failed to find preference value for key '%s'\n"
                          "Using empty string.  This is probably wrong!\n",
                          key);
                return g_strdup("");
        }
        return value;
}



/* Wrapper to get integer preference values. */
int pref_get_int_value(char *key)
{
        g_assert(key);
        return const_get_int(key);
}



/* Get a pref value, default to deflt if no value has been
   specified. */
char *pref_get_value_silent_default(char *key, const char *deflt)
{
        char *value;

        g_assert(key);
        g_assert(deflt);
        value = const_get_string(key);
        if(!value) {
                return g_strdup(deflt);
        }
        return value;
}



/* Wrapper to get integer preference values without error if the value
   is not available.

   ### We can't distinguish between key not found and key == 0. ###
*/
int pref_get_int_value_silent_default(char *key, const char *deflt)
{
        g_assert(key);
        g_assert(deflt);
        return const_get_int(key);
}


