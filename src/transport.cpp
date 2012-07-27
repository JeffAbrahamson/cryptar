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

#include "transport.h"


using namespace cryptar;
using namespace std;



/*
  Factory method to build transport objects from transport types.
*/
Transport *cryptar::make_transport(TransportType in_transport_type, const Config &in_config)
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
        case rsync_push:
                return new TransportRsyncPush(in_config);
        case rsync_pull:
                return new TransportRsyncPull(in_config);
        }
        ostringstream error_message;
        error_message << "Unknown staging type, " << in_transport_type;
        throw(runtime_error(error_message.str()));
}


Transport::Transport(const Config &in_config)
{
}


/*
TransportRsyncPush::TransportRsyncPush(const Config &in_config)
{
}
*/
