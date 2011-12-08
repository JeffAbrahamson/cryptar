/*  env.c
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
#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"

#include "env.h"
#include "prefs.h"
#include "syscall.h"


static void setup_config_dir();
static char *get_config_dir_name();
static void set_config_dir(const char *dir);
static const char *get_config_dir();








void setup_env(const char *env_name)
{
        assert(env_name);
        if(!get_config_dir())
                setup_config_dir();
        /* setup_dir */
        return;
}




static void setup_config_dir()
{
        int ret;
        struct stat statbuf;
        char *config_dir = get_config_dir_name();


        if((ret = stat(config_dir, &statbuf))) {
                if(errno != ENOENT) {
                        g_warning("The configuration directory %s doesn't work for me.\n(I can't even create it.) Exiting.", config_dir);
                        exit(-1);
                }
                /* Directory doesn't exist, so create it. */
                /* Ideally, someday, create recursively. */
                if(do_mkdir(config_dir, 0700) < 0) {
                        g_warning("Can't create configuration directory %s:\n%d: %s\nExiting.", config_dir, errno, strerror(errno));
                        exit(-1);
                }
        }
        set_config_dir(config_dir);
        return;
}



/* Note that result must be deallocated by caller.
 */
static char *get_config_dir_name()
{
        char *config_dir, *full_config_dir;
        char *home;
        
        if(!(config_dir = getenv(CONFIG_DIR_VARNAME)))
                config_dir = CONFIG_DIR;
        assert(config_dir);
        if(config_dir[0] == '/')
                return g_strdup(config_dir);
        
        home = getenv("HOME");
        if(!home) {
                g_warning("Configuration directory %s is not\nan absolute path, but $HOME is not defined.\nExiting.", config_dir);
                exit(-1);
        }
        full_config_dir = g_strdup_printf("%s/%s", home, config_dir);
        return full_config_dir;
}




static char *config_dir;

static void set_config_dir(const char *dir)
{
        assert(dir);
        config_dir = g_strdup(dir);
}



static const char *get_config_dir()
{
        return config_dir;
}
