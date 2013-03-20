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



#ifndef __CRYPTAR_H__
#define __CRYPTAR_H__ 1


#include <list>
#include <map>
#include <string>
#include <vector>

#include "compress.h"
#include "crypt.h"
#include "mode.h"
#include "block.h"
#include "config.h"
#include "transport.h"
#include "communicate.h"
#include "filesystem.h"
#include "act.h"


namespace cryptar {

        
        /* ************************************************************ */
        /* work units */

        /*
          A work unit is something that can be passed between a work
          thread and a communication thread.  The contract here is
          that the communication thread has control of the structure
          from the time the client calls hand_to_comm() until
          comm_done() returns true.

          This is probably obsolete, I think I rather want to derive
          from Block and pass on queues.
        */

        class WorkUnit {

        public:
                WorkUnit() {};
                virtual ~WorkUnit() {};

                void hand_to_comm() { m_comm_done = false; }  // should do handoff, too
                void comm_done() { m_comm_done = true; }    // should note in done queue
                bool is_comm() { return !m_comm_done; }

        private:
                // true if client has control, false if belongs to
                // communication thread
                bool m_comm_done;

                // Also needs reference or pointer to thread
                // and to (mutex-protected?) queue of things that are
                // done.
        };
}

#endif  /* __CRYPTAR_H__*/
