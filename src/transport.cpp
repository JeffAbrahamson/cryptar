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


#include <fstream>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "crypt.h"
#include "system.h"
#include "transport.h"


using namespace cryptar;
using namespace std;


#if 0
/*
  Factory method to build transport objects from transport types.
*/
Transport *cryptar::make_transport(TransportType in_transport_type,
                                   const shared_ptr<Config> in_config)
{
        switch(in_transport_type) {
        case invalid_transport:
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
                return new TransportFS(in_config);
        }
        ostringstream error_message;
        error_message << "Unknown staging type, " << in_transport_type;
        throw(runtime_error(error_message.str()));
}


Transport *cryptar::make_transport(const std::shared_ptr<Config> config)
{
        return make_transport(no_transport, config);
}

NoTransport *cryptar::make_no_transport(const std::shared_ptr<Config> config)
{
        return dynamic_cast<NoTransport *>(make_transport(no_transport, config));
}


TransportFS *cryptar::make_transport_fs(const std::shared_ptr<Config> config)
{
        return dynamic_cast<TransportFS *>(make_transport(fs, config));
}
#endif


/*
*/
TransportFS::TransportFS(const string &in_base_path)
        : Transport(), m_base_path(in_base_path)
{
        // base path, if provided, must end in '/'
        if(!m_base_path.empty())
                //assert('/' == *(m_base_path.end()--)); // FIXME    (OS-specific)
                assert('/' == m_base_path.back()); // FIXME    (OS-specific)
        if(!m_base_path.empty() && mkdir(m_base_path.c_str(), 0700) && EEXIST != errno)
                throw_system_error("TransportFS::TransportFS()");
                // FIXME:  should try harder (mkdir -p)
}



/*
  Return the name of the file in which this block's content should be
  stored.
*/
const string TransportFS::block_to_filename(const Block *in_block) const
{
        return m_base_path + message_digest(in_block->id().as_string(), true);
}


void TransportFS::read(Block *in_block) const
{
        string filename(block_to_filename(in_block));
        ifstream fs(filename, ios_base::binary);
        if(!fs) {
                const size_t len = 1024; // arbitrary
                char errstr[len];
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
                perror(0);
                cout << "(just called perror(0))" << "  errno=" << errno << endl;
                cout << "filename=" << filename << endl;
                if(strerror_r(errno, errstr, len))
                        throw("strerror_r() error in Block::read() error");
#else
                strerror_r(errno, errstr, len);
#endif
                cerr << "Block read error: " << errstr << endl;
                throw("Block::read()");
        }
        fs.seekg(0, ios::end);
        int length = fs.tellg();
        fs.seekg(0, ios::beg);
        char *buffer = new char[length];
        fs.read(buffer, length);
        string payload(buffer, length);
        in_block->from_stream(payload);
        fs.close();
}


void TransportFS::write(const Block *in_block) const
{
        const string filename(block_to_filename(in_block));
        const string payload(in_block->to_stream());
        ofstream fs(filename, ios_base::binary | ios_base::trunc);
        fs.write(payload.data(), payload.size());
        if(!fs)
                throw_system_error("TransportFS::write()");
        fs.close();
}



#if 0    // for NetTransport
/*
  Provide a pointer to the receive queue (the object that requests
  data from the remote store).
*/
shared_ptr<Communicator> Config::receiver()
{
        assert(m_receiver);
        return m_receiver;
}


/*
  Provide a pointer to the send queue (the object that pushes data to
  the remote store).
*/
shared_ptr<Communicator> Config::sender()
{
        assert(m_sender);
        return m_sender;
}
#endif


/*
  Serialize or deserialize according to context.
*/
template<class Archive>
void Transport::serialize(Archive &in_ar, const unsigned int in_version)
{
        //in_ar & m_dbs;
}

