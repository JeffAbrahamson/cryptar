/*  ios.h
 *  Copyright (C) 2002-2003 by:
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


#ifndef __IOS_H__
#define __IOS_H__

#include <glib.h>

typedef struct ios_buf IOSBuf;

IOSBuf *ios_new();
IOSBuf *ios_new_copy(IOSBuf *ios_src);
void ios_free(struct ios_buf *ios);
void ios_grow(struct ios_buf *ios, guint32 size);
void ios_reset(struct ios_buf *ios);
void ios_append(struct ios_buf *ios, const void *data, int size);
void ios_append_ios(IOSBuf *ios, IOSBuf *ios_src);
void ios_append_int8(IOSBuf *ios, gint8 num);
void ios_append_int16(struct ios_buf *ios, gint16 num);
void ios_append_int32(struct ios_buf *ios, gint32 num);
void ios_append_string(struct ios_buf *ios, const char *str);
void *ios_at(struct ios_buf *ios, guint32 offset);
void *ios_read(IOSBuf *ios, guint32 len);
guint16 ios_int16_at(IOSBuf *ios, guint32 offset);
guint16 ios_read_int16(IOSBuf *ios);
guint32 ios_int32_at(IOSBuf *ios, guint32 offset);
guint32 ios_read_int32(IOSBuf *ios);
char *ios_read_str(struct ios_buf *ios);
void *ios_buffer(IOSBuf *ios);
guint32 ios_buffer_size(IOSBuf *ios);
void ios_set_buffer(IOSBuf *ios, const void *data, guint32 size);
void ios_copy_ios(IOSBuf *ios_new, IOSBuf *ios_src);

#endif
