/*  log.c
 *  Copyright (C) 2002, 2003 by:
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


#include <glib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"


enum log_entity {
        kLog_none,
        kLog_client,
        kLog_server
};

static enum log_entity log_entity_flag = kLog_none;


void log_am_client()
{
        log_entity_flag = kLog_client;
        return;
}



void log_am_server()
{
        log_entity_flag = kLog_server;
        return;
}



/* This is essentially a copy of g_log_default_handler from
   gmessages.c in glib-1.2.
 */
void log_default_log_handler (const gchar    *log_domain,
                              GLogLevelFlags  log_level,
                              const gchar    *message,
                              gpointer        unused_data)
{
#ifdef NATIVE_WIN32
        FILE *fd;
#else
        gint fd;
#endif
        gboolean in_recursion;
        gboolean is_fatal;  

        if(!log_domain)
                switch(log_entity_flag) {
                case kLog_none: log_domain = ""; break;
                case kLog_client: log_domain = "client"; break;
                case kLog_server: log_domain = "  server"; break;
                default: log_domain = " ???";
                }
               
        unused_data = NULL;     /* unused: avoid compiler warnings */
        in_recursion = (log_level & G_LOG_FLAG_RECURSION) != 0;
        is_fatal = (log_level & G_LOG_FLAG_FATAL) != 0;
        log_level &= G_LOG_LEVEL_MASK;
  
        if (!message)
                message = "g_log_default_handler(): (NULL) message";
  
#ifdef NATIVE_WIN32
        /* Use just stdout as stderr is hard to get redirected from the
         * DOS prompt.
         */
        fd = stdout;
#else
        fd = STDERR_FILENO;
#endif
  
        switch (log_level) {
        case G_LOG_LEVEL_ERROR:
                if (log_domain) {
                        write (fd, "\n", 1);
                        write (fd, log_domain, strlen (log_domain));
                        write (fd, "-", 1);
                } else
                        write (fd, "\n** ", 4);
                if (in_recursion)
                        write (fd, "ERROR (recursed) **: ", 21);
                else
                        write (fd, "ERROR **: ", 10);
                write (fd, message, strlen(message));
                if (is_fatal)
                        write (fd, "\naborting...\n", 13);
                else
                        write (fd, "\n", 1);
                break;
        case G_LOG_LEVEL_CRITICAL:
                if (log_domain) {
                        write (fd, "\n", 1);
                        write (fd, log_domain, strlen (log_domain));
                        write (fd, "-", 1);
                } else
                        write (fd, "\n** ", 4);
                if (in_recursion)
                        write (fd, "CRITICAL (recursed) **: ", 24);
                else
                        write (fd, "CRITICAL **: ", 13);
                write (fd, message, strlen(message));
                if (is_fatal)
                        write (fd, "\naborting...\n", 13);
                else
                        write (fd, "\n", 1);
                break;
        case G_LOG_LEVEL_WARNING:
                if (log_domain) {
                        write (fd, "\n", 1);
                        write (fd, log_domain, strlen (log_domain));
                        write (fd, "-", 1);
                } else
                        write (fd, "\n** ", 4);
                if (in_recursion)
                        write (fd, "WARNING (recursed) **: ", 23);
                else
                        write (fd, "WARNING **: ", 12);
                write (fd, message, strlen(message));
                if (is_fatal)
                        write (fd, "\naborting...\n", 13);
                else
                        write (fd, "\n", 1);
                break;
        case G_LOG_LEVEL_MESSAGE:
                if (log_domain) {
                        write (fd, log_domain, strlen (log_domain));
                        write (fd, "-", 1);
                }
                if (in_recursion)
                        write (fd, "Message (recursed): ", 20);
                else
                        write (fd, "Message: ", 9);
                write (fd, message, strlen(message));
                if (is_fatal)
                        write (fd, "\naborting...\n", 13);
                else
                        write (fd, "\n", 1);
                break;
        case G_LOG_LEVEL_INFO:
                if (log_domain) {
                        write (fd, log_domain, strlen (log_domain));
                        write (fd, "-", 1);
                }
                if (in_recursion)
                        write (fd, "INFO (recursed): ", 17);
                else
                        write (fd, "INFO: ", 6);
                write (fd, message, strlen(message));
                if (is_fatal)
                        write (fd, "\naborting...\n", 13);
                else
                        write (fd, "\n", 1);
                break;
        case G_LOG_LEVEL_DEBUG:
                if (log_domain) {
                        write (fd, log_domain, strlen (log_domain));
                        write (fd, "-", 1);
                }
                if (in_recursion)
                        write (fd, "DEBUG (recursed): ", 18);
                else
                        write (fd, "DEBUG: ", 7);
                write (fd, message, strlen(message));
                if (is_fatal)
                        write (fd, "\naborting...\n", 13);
                else
                        write (fd, "\n", 1);
                break;
        default:
                /* we are used for a log level that is not
                 * defined by GLib itself, try to make the
                 * best out of it.
                 */
                if (log_domain) {
                        write (fd, log_domain, strlen (log_domain));
                        if (in_recursion)
                                write (fd, "-LOG (recursed:", 15);
                        else
                                write (fd, "-LOG (", 6);
                } else if (in_recursion)
                        write (fd, "LOG (recursed:", 14);
                else
                        write (fd, "LOG (", 5);
                if (log_level) {
                        gchar string[] = "0x00): ";
                        gchar *p = string + 2;
                        guint i;
	  
                        i = g_bit_nth_msf (log_level, -1);
                        *p = i >> 4;
                        p++;
                        *p = '0' + (i & 0xf);
                        if (*p > '9')
                                *p += 'A' - '9' - 1;
	  
                        write (fd, string, 7);
                } else
                        write (fd, "): ", 3);
                write (fd, message, strlen(message));
                if (is_fatal)
                        write (fd, "\naborting...\n", 13);
                else
                        write (fd, "\n", 1);
                break;
        }
}
