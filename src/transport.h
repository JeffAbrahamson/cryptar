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
          The base transport class does nothing (i.e., no transport).
          Nonetheless, for testing we'll trivially derive from the
          base class so that a programming error that slices to the
          base class will still be caught.
        */
        class Transport {
        protected:
                Transport(const std::shared_ptr<Config> in_config) {};
                friend Transport *cryptar::make_transport(TransportType in_transport_type,
                                                          const std::shared_ptr<Config> in_config);
                
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
                  An action to transport a block.
                */
                virtual void operator()(Block *bp) const {};
                
                /*
                  An action to take after all blocks are transported.
                  If we have a network connection to the remote
                  store, this is where we tear it down.
                */
                virtual void post() const {};
                
        //protected:
                //std::shared_ptr<Config> m_config;
        };

        Transport *make_transport(const std::shared_ptr<Config> config);
        

        /*
          Transport class that does nothing.
          Not very different from the base class, but signals
          that we mean to do nothing rather than a bug that sliced
          us to the base class.
        */
        class NoTransport : public Transport {
        protected:
                NoTransport(const std::shared_ptr<Config> in_config)
                        : Transport(in_config) {};
                friend Transport *cryptar::make_transport(TransportType in_transport_type,
                                                          const std::shared_ptr<Config> in_config);

        public:
                virtual TransportType transport_type() { return no_transport; }

                virtual void pre() const {};
                virtual void operator()(Block *bp) const {};
                virtual void post() const {};
        };

        NoTransport *make_no_transport(const std::shared_ptr<Config> config);


        /*
          Write blocks to the file system.
          The config will tell us where to write them.
          The simplest use of TransportFSOut is when the
          remote store is a remote file system.
         */
        class TransportFSOut : public Transport {
        protected:
                TransportFSOut(const std::shared_ptr<Config> in_config);
                friend Transport *cryptar::make_transport(TransportType in_transport_type,
                                                          const std::shared_ptr<Config> in_config);

        public:
                virtual ~TransportFSOut() {};

                virtual TransportType transport_type() { return fs_out; }

                virtual void pre() const {};
                virtual void operator()(Block *bp) const;
                virtual void post() const {};
        private:
                const std::string m_local_dir;
        };

        TransportFSOut *make_transport_fsout(const std::shared_ptr<Config> config);

        
        /*
          Read blocks from the file system.
          The config will tell us where to find them.
          The simplest use of TransportFSIn is when the
          remote store is a remote file system.
         */
        class TransportFSIn : public Transport {
        protected:
                TransportFSIn(const std::shared_ptr<Config> in_config);
                friend Transport *cryptar::make_transport(TransportType in_transport_type,
                                                          const std::shared_ptr<Config> in_config);

        public:
                virtual ~TransportFSIn() {};

                virtual TransportType transport_type() { return fs_in; }

                virtual void pre() const {};
                virtual void operator()(Block *bp) const;
                virtual void post() const {};
        private:
                const std::string m_local_dir;
        };

        TransportFSIn *make_transport_fsin(const std::shared_ptr<Config> config);

        
        // FIXME:  We'll also want TransportTLSIn and TransportTLSOut.
        //         They will require a wire protocol.  Cf. ../dev/comm.txt.
        
}


#endif  /* __TRANSPORT_H__*/
