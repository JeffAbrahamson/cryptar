/*
  Copyright 2012  Jeff Abrahamson
  
  This file is part of crytpar.
  
  crytpar is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  crytpar is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with crytpar.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "transport.h"


using namespace cryptar;
using namespace std;



/*
  Factory method to build transport objects from transport types.
*/
Transport *cryptar::make_transport(TransportType in_transport_type, const shared_ptr<Config> in_config)
{
        switch(in_transport_type) {
        case transport_invalid:
                {
                        // Why do I see an error if this block is not separately scoped?
                        ostringstream error_message("Unexpected transport type, ");
                        error_message << in_transport_type;
                        throw(runtime_error(error_message.str()));
                }
        case base_transport:
                {
                        // Why do I see an error if this block is not separately scoped?
                        ostringstream error_message("Unexpected transport type, ");
                        error_message << in_transport_type;
                        error_message << " (Attempt to instantiate base class.)";
                        throw(runtime_error(error_message.str()));
                }
        case no_transport:
                return new NoTransport(in_config);
        case fs:
                throw(runtime_error("Transport type 'fs' is not valid here."));
        case fs_out:
                return new TransportFSOut(in_config);
        case fs_in:
                return new TransportFSIn(in_config);
        }
        ostringstream error_message;
        error_message << "Unknown staging type, " << in_transport_type;
        throw(runtime_error(error_message.str()));
}


// FIXME  FSIn and FSOut are similar, refactor together


/*
  Directory where we should stage files.
*/
TransportFSOut::TransportFSOut(const shared_ptr<Config> in_config)
        : Transport(in_config), m_local_dir(in_config->local_dir())
{
        if(mkdir(m_local_dir.c_str(), 0700) && EEXIST != errno) {
                cerr << "  Error creating directory \""
                     << m_local_dir << "\": " << strerror(errno) << endl;
                throw(runtime_error("Failed to create staging directory."));
                // FIXME:  should try harder (mkdir -p)
        }
}



void TransportFSOut::operator()(Block *bp) const
{
        assert(bp);
        bp->write(m_local_dir);
}



/*
  Directory where we should stage files.
*/
TransportFSIn::TransportFSIn(const shared_ptr<Config> in_config)
        : Transport(in_config), m_local_dir(in_config->local_dir())
{
        if(mkdir(m_local_dir.c_str(), 0700) && EEXIST != errno) {
                cerr << "  Error creating directory \""
                     << m_local_dir << "\": " << strerror(errno) << endl;
                throw(runtime_error("Failed to create staging directory."));
                // FIXME:  should try harder (mkdir -p)
        }
}



void TransportFSIn::operator()(Block *bp) const
{
        assert(bp);
        bp->read(m_local_dir);
}
