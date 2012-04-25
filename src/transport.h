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



#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__ 1


#include "block.h"
#include "config.h"


namespace cryptar {

        /*
          Staging involves persisting (or reading) a block to (or
          from) a local file.

          Transporting involves moving data to or from a remote host.

          The initial rsync-based mechanism writes files to a local
          staging directory before rsyncing them to the remote host.
          Other remote arrangements, such as various cloud providers,
          might involve different staging or transport mechanisms.
          Either (but not both of) staging or transport may be a
          no-op.
        */

        
        /*
          The base transport class does nothing (i.e., no transport).
          This corresponds to using cryptar on a local store, as a
          sort of encrypted svn repository.  More usefully, it helps
          for testing.  Otherwise, a derived class is of more
          interest.
        */
        class Transport {
        public:
                Transport(const Config &in_config);
                virtual ~Transport() {};

                // An action to take before transporting anything.
                virtual void pre() const {};
                // An action to transport a block
                virtual void operator()(Block *bp) const {};
                // An action to take after all blocks are transported.
                virtual void post() const {};
                
        protected:
                Config m_config;
        };


        class TransportRsyncPush : public Transport {
        public:
                TransportRsyncPush(const Config &config)
                        : Transport(config) {};
                virtual ~TransportRsyncPush() {};

                // An action to take before transporting anything.
                virtual void pre() const {};
                // An action to transport a block
                virtual void operator()(Block *bp) const {};
                // An action to take after all blocks are transported.
                virtual void post() const {};
        };
        

        class TransportRsyncPull : public Transport {
        public:
                TransportRsyncPull(const Config &config)
                        : Transport(config) {};
                virtual ~TransportRsyncPull() {};

                // An action to take before transporting anything.
                virtual void pre() const {};
                // An action to transport a block
                virtual void operator()(Block *bp) const {};
                // An action to take after all blocks are transported.
                virtual void post() const {};
        };

}


#endif  /* __TRANSPORT_H__*/
