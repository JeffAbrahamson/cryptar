/*
  Copyright 2012  Jeff Abrahamson
  
  This file is part of cryptar.
  
  cryptar is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  cryptar is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with cryptar.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef __COMMUNICATE_H__
#define __COMMUNICATE_H__ 1


#include <boost/thread.hpp>
#include <queue>

#include "block.h"
#include "stage.h"
#include "transport.h"


namespace cryptar {

        const int communicator_prod_batch_size = 100;
        const int communicator_test_batch_size = 3;

        class Communicator {
        public:
                Communicator(const Stage *in_stage,
                             const Transport *in_transport);
                ~Communicator();

                void push(Block *);
                void wait();
                void operator()();   // Should really only be called at thread creation

        private:
                typedef std::queue<Block *>::size_type queue_size_type;
                
                void run();
                bool queue_empty();
                Block *pop();
                void comm_batch();

                const queue_size_type m_batch_size;
                /*
                  If these next are values instead of pointer (or
                  reference, but that makes instantiating this
                  annoying), then we slice the objects to the base.
                  So just use pointers.
                */
                const Stage *m_stage;
                const Transport *m_transport;

                bool m_needed;    /* set to false to encourage auto-shutdown */
                boost::mutex m_queue_access;
                std::queue<Block *> m_queue;
                boost::thread *m_thread;
        };
}


#endif  /* __COMMUNICATE_H__*/
