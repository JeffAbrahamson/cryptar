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


#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <glib.h>


enum int_options {
        kOption_backup,
        kOption_create,
        kOption_extract,
        kOption_list,
        kOption_server,
        kOption_ping,
        kOption_verbose,
        kOption_dryrun,
        kOption_read_only,
        kOption_list_only,
#ifdef MAINTAINER_MODE
        kOption_self_test,
#endif
        
        /* last value */
        kNum_int_options
};

/*
  The various pattern options (backup, extract, list) are mutually
  exclusive, but it's less confusing to have several names than one
  name that gets used no matter what we're doing.
*/
enum char_options {
        kArchive_target,     /* the archive (database) to work with */
        kBackup_pattern, /* pattern to backup (subset of configured files) */
        kExtract_pattern,       /* pattern to restore */
        kList_pattern,          /* pattern to list */

        /* last value */
        kNum_char_options
};

/* Bit-field options for verbose flag */
#define VERBOSE_NONE		0
#define VERBOSE_FILES		1 /* what files we're working on */
#define VERBOSE_PROTOCOL 	2 /* wire protocol stages */
#define VERBOSE_IO		4 /* bits transferred */
#define VERBOSE_IO_SUM		8 /* I/O summary info */
#define VERBOSE_FLOW		16 /* interesting code points */
#define VERBOSE_FLOW_PLUS 	32 /* extra data about where code points */
#define VERBOSE_DB		64 /* database information */

extern GPtrArray *g_args;


void options_init();
void set_int_option(enum int_options opt, int val);
int int_option(enum int_options opt);
void set_char_option(enum char_options opt, char *val);
char *char_option(enum char_options opt);
char *char_option_safe(enum char_options opt);

#endif /* __OPTIONS_H__ */
