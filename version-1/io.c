/*  io.c
 *  Copyright (C) 2002, 2003, 2004 by:
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


#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"

#include "io.h"
#include "ios.h"
#include "main.h"
#include "option.h"
#include "options.h"
#include "protocol.h"
#include "workticket.h"



/* Apparatus for the glib event handlers.  Instead of dealing with
   input and output file descriptors, we can let glib take care of the
   polling and so forth and just do our thing when there's something
   to do.
 */


static int ignore_input = 0;        /* when set to 1, ignore input forever more */
static int ignore_output = 0;        /* when set to 1, refuse output forever more */
static IOSBuf *ios_write_buf = NULL;
static IOSBuf *ios_read_buf = NULL;

static GIOChannel *io_chan_in = NULL;
static GIOChannel *io_chan_out = NULL;

static gboolean io_output_handler(GIOChannel *ioch, GIOCondition cond, void *data);
static gboolean io_input_handler(GIOChannel *ioch, GIOCondition cond, void *data);
static int io_cond_ok(GIOCondition cond);



/* If someone asks for the output iosbuf and the output channel isn't
   registered, register it. The output handler will deregister itself
   when there's no work to do in order to avoid spin loops.
 */
IOSBuf *io_write_buf()
{
        if(!ios_write_buf)
                ios_write_buf = ios_new();
        io_write_reg();
        return ios_write_buf;
}



/* Register the output channel.
 */
void io_write_reg()
{
        if(io_chan_out->ref_count == 1) {
                if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                        g_message("Registering output channel in io_write_reg");
                g_io_add_watch(io_chan_out,
                               G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
                               io_output_handler, ios_write_buf);
        }
        return;
}



/* Return the input channel.
 */
IOSBuf *io_read_buf()
{
        if(!ios_read_buf)
                ios_read_buf = ios_new();
        return ios_read_buf;
}



/* Queue an iosbuf to send across the wire. */
void io_send_ios(IOSBuf *ios)
{
        IOSBuf *out;
        
        g_assert(ios);
        if(ignore_output) {
                g_warning("Ignoring output, but got some anyway.  So ignoring it.");
                return;
        }
        if(int_option(kOption_verbose) & VERBOSE_IO_SUM)
                g_message("Sending %d + 4 bytes.", ios_buffer_size(ios));
        out = io_write_buf();
        ios_append_int32(out, ios_buffer_size(ios));
        ios_append(out, ios_buffer(ios), ios_buffer_size(ios));
        return;
}



/* Irrevocably stop listening to input, we're done. */
void io_no_more_input()
{
        ignore_input = 1;
        return;
}



/* Irrevocably stop accepting data to output, we're done. */
void io_no_more_output()
{
        ignore_output = 1;
        return;
}



/* Tell the glib event loop about our input and output descriptors.
 */
void io_register_fd_pair(int fd_in, int fd_out)
{
        IOSBuf *ios_in, *ios_out;
        guint in_id;            /* probably ignored */
        int other;

        other = G_IO_ERR | G_IO_HUP | G_IO_NVAL;
        
        ios_in = io_read_buf();
        io_chan_in = g_io_channel_unix_new(fd_in);
        in_id = g_io_add_watch(io_chan_in,
                               G_IO_IN | other,
                               io_input_handler,
                               ios_in);
        
        io_chan_out = g_io_channel_unix_new(fd_out);
        ios_out = io_write_buf();
        /* the output buffer will add this for us */
        
        return;
}



/* Event loop callback for handling output.  This is called if the
   output file descriptor is able to accept data.  To avoid spin loops
   where we're called even though we have no data to send, we
   deregister the output channel when we have nothing to say, then
   reregister it when we have something to say again.
   
   Return 0 to be deregistered and removed from the event list,
   return non-zero to stay registered.
*/
static gboolean io_output_handler(GIOChannel *ioch, GIOCondition cond, void *data)
{
        IOSBuf *ios;
        GIOError err;
        guint bytes_written;

        assert(ioch);
        assert(data);
        ios = (IOSBuf *)data;
        g_assert(cond != G_IO_IN);
        if(io_cond_ok(cond)) {
                if(int_option(kOption_verbose) & VERBOSE_FLOW)
                        g_message("Deregistering output handler and signaling quit");
                quit_main_loop();
                return 0;
        }

        if(!ios_buffer_size(ios))
                wt_process_queue(ios);

        if(!ios_buffer_size(ios)) {
                if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                        g_message("Nothing to write, deregistering output handler");
                if(ignore_output) {
                        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                                g_message("Ignoring output and queue is empty.");
                        quit_main_loop();
                }
                return 0;
        }
        
        err = g_io_channel_write(ioch, ios_buffer(ios),
                                 ios_buffer_size(ios), &bytes_written);
        if(err != G_IO_ERROR_NONE) {
                g_warning("io_output_handler reporting error %d", err);
                if(errno)
                        perror("io_output_handler: ");
                return 0;
        }
        assert(ios_read(ios, bytes_written));
        if(int_option(kOption_verbose) & VERBOSE_IO_SUM)
                g_message("Wrote %d bytes.", bytes_written);
        
        return 1;               /* true means don't remove */
}



#define BUF_SIZE 8196

/* Event loop callback for input.  See the comment at
   io_output_handler for details.
   
   Return 0 to be deregistered and removed from the event list,
   non-zero to stay registered.
*/
static gboolean io_input_handler(GIOChannel *ioch, GIOCondition cond, void *data)
{
        IOSBuf *ios;
        GIOError err;
        guint bytes_read;
        char buf[BUF_SIZE];
        
        assert(ioch);
        assert(data);
        ios = (IOSBuf *)data;
        g_assert(cond != G_IO_OUT);
        if(ignore_input)
                return 0;
        if(io_cond_ok(cond)) {
                if(int_option(kOption_verbose) & VERBOSE_FLOW)
                        g_message("Deregistering input handler and signaling quit");
                quit_main_loop();
                return 0;
        }
        
        err = g_io_channel_read(ioch, buf, BUF_SIZE, &bytes_read);
        ios_append(ios, buf, bytes_read);
        if(err != G_IO_ERROR_NONE) {
                g_warning("io_input_handler reporting error %d", err);
                if(errno)
                        perror("io_input_handler: ");
        }
        if(int_option(kOption_verbose) & VERBOSE_IO_SUM)
                g_message("Read %d bytes.", bytes_read);
        if(!bytes_read) {
                /* We shouldn't have been scheduled if there was
                   nothing to read, so sleep for one second to avoid a
                   spin loop. */
                sleep(1);
        }

        /* If dispatcher returns non-zero, we'll remove this input
           source. */
        if(read_and_dispatch_commands(ios)) {
                if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                        g_message("Deregistering input handler.");
                return 0;
        }
        return 1;
}



/* Close both IO channels.
 */
void io_close_io()
{
        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("closing I/O channels");
        g_io_channel_close(io_chan_in);
        g_io_channel_close(io_chan_out);
        return;
}



/* Return 0 if the channel is not in an error state.
 */
static int io_cond_ok(GIOCondition cond)
{
        if(cond & G_IO_ERR) {
                if(int_option(kOption_verbose) & VERBOSE_FLOW)
                        g_message("io_input_handler: cond == G_IO_ERR");
                return 1;
        }
        if(cond & G_IO_HUP) {
                if(int_option(kOption_verbose) & VERBOSE_FLOW)
                        g_message("io_input_handler: cond == G_IO_HUP");
                return 1;
        }
        if(cond & G_IO_NVAL) {
                if(int_option(kOption_verbose) & VERBOSE_FLOW)
                        g_message("io_input_handler: cond == G_IO_OUT");
                return 1;
        }

        return 0;
}

