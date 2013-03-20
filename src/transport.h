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


#include <memory>

#include "block.h"
#include "config.h"


namespace cryptar {

        class Transport;
        Transport *make_transport(TransportType in_transport_type,
                                  const std::shared_ptr<Config> in_config);

        /*
          A Transport tells us how to access a store.

          The access might be via the local filesystem, or it might be
          network-based.  Network-based stores could, in principle,
          have different storage and access models.  For example, we
          might have a custom network store for cryptar, but then also
          a network store that works with AWS or that uses scp
          transfer.

          It contains some parameters and exports a rather simple
          interface.  It may be synchronous (filesystem or network in
          test mode) or asynchronous.
        */

        /*
          The base transport class does nothing (i.e., no transport).
          Nonetheless, for testing we'll trivially derive from the
          base class so that a programming error that slices to the
          base class will still be caught.
        */
        class Transport {
        protected:
                Transport() {};
                // FIXME    (Config constructor unneeded?)
                /*
                Transport(const std::shared_ptr<Config> in_config) {};
                friend Transport *cryptar::make_transport(TransportType in_transport_type,
                                                          const std::shared_ptr<Config> in_config);
                */
                
        public:
                virtual ~Transport() {};

                virtual TransportType transport_type() { return base_transport; }

                /*
                  An action to take before transporting anything.
                  If we have a network connection to the remote
                  store, this is where we set up the connection.
                */
                virtual void pre() const {};
                
                /*
                  An action to read or write data.
                  For file transport, this is likely a synchronous read or write.
                  For network transport, this is likely a queuing operation
                  (for read, queuing a request for in_block->id).
                  Either way, the Block's ACT probably sets status
                  (in the network case, once we get the ACK back).
                */
                virtual void read(Block *in_block) const = 0;
                virtual void write(const Block *in_block) const = 0;
                
                /*
                  An action to take after all blocks are transported.
                  If we have a network connection to the remote
                  store, this is where we tear it down.
                */
                virtual void post() const {};
                
        private:
                friend class boost::serialization::access;
                template<class Archive>
                        void serialize(Archive &ar, const unsigned int version);
        };

        //Transport *make_transport(const std::shared_ptr<Config> config);
        

        /*
          Transport class that does nothing.
          Not very different from the base class, but signals
          that we mean to do nothing rather than a bug that sliced
          us to the base class.
        */
        class NoTransport : public Transport {
        protected:
                /*
                NoTransport(const std::shared_ptr<Config> in_config)
                        : Transport(in_config) {};
                friend Transport *cryptar::make_transport(TransportType in_transport_type,
                                                          const std::shared_ptr<Config> in_config);
                */
                
        public:
                virtual TransportType transport_type() { return no_transport; }

                virtual void read(Block *in_block) const {};
                virtual void write(const Block *in_block) const {};
        };

        //NoTransport *make_no_transport(const std::shared_ptr<Config> config);


        /*
          Write blocks to the file system.
          The config will tell us where to write them.
          The simplest use of TransportFSOut is when the
          remote store is a remote file system.
         */
        class TransportFS : public Transport {
        protected:
                /*
                TransportFS(const std::shared_ptr<Config> in_config);
                friend Transport *cryptar::make_transport(TransportType in_transport_type,
                                                          const std::shared_ptr<Config> in_config);
                */
        public:
                TransportFS(const std::string &in_base_path);
                virtual ~TransportFS() {};

                virtual TransportType transport_type() { return fs; }

                virtual void read(Block *in_block) const;
                virtual void write(const Block *in_block) const;

        private:
                const std::string block_to_filename(const Block *in_block) const;
                
                std::string m_base_path;
        };

        //TransportFS *make_transport_fs(const std::shared_ptr<Config> config);

        
        // FIXME:  We'll also want TransportTLSIn and TransportTLSOut.
        //         They will require a wire protocol.  Cf. ../dev/comm.txt.
        
}


#if 0
                // From Config.  Belongs in NetTransport
                std::shared_ptr<Communicator> m_receiver;
                std::shared_ptr<Communicator> m_sender;
                
                std::shared_ptr<Communicator> receiver();
                std::shared_ptr<Communicator> sender();
#endif


#endif  /* __TRANSPORT_H__*/
