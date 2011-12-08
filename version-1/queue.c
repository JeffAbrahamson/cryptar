/*  queue.c
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


#include <assert.h>

#include "queue.h"


Queue *new_queue()
{
        Queue *q;

        q = g_new(Queue, 1);
        q->head = NULL;
        return q;
}



void enqueue(Queue *q, void *data)
{
        assert(q);
        assert(data);
        
        q->head = g_slist_append(q->head, data);
        return;
}


void *dequeue(Queue *q)
{
        GSList *gsl;
        void *data;
        
        assert(q);
        if(q->head == NULL)
                return NULL;
        gsl = q->head;
        data = gsl->data;
        q->head = g_slist_remove(q->head, q->head->data);
        assert(q->head != gsl);
        return data;
}

