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


#include <assert.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "compress.h"
#include "config.h"
#include "crypt.h"
#include "mode.h"
#include "system.h"
#include "transport.h"


using namespace cryptar;
using namespace std;


/*
  Return a transport, possibly constructing it.
*/
const shared_ptr<Transport> ConfigParam::transport() const
{
        if(!m_transport)
                m_transport = make_transport();
        return m_transport;
}


/*
  Private function to construct a transport.

  All callers should use public function except, of course, the public
  function itself.
*/
const shared_ptr<Transport> ConfigParam::make_transport() const
{
        if(invalid_transport == m_transport_type)
                throw(runtime_error("Invalid transport"));
        if(no_transport == m_transport_type)
                return make_shared<NoTransport>();
        if(fs == m_transport_type)
                return make_shared<TransportFS>(m_local_dir);
        	//return shared_ptr<TransportFS>(new TransportFS(m_local_dir));
        throw(runtime_error("Unknown transport"));
}



#if 0
/*
  Cf. comments at Config::Config below.

  We can make new configs by specifying a filename and passphrase, in
  which case we load the named file, decrypt using the passphrase, and
  rehydrate the config.  The file contains the information that we
  would have provided in a ConfigParam instance (below).

  Alternately, we can instantiate with a ConfigParams structure, which
  lets us create a new and as yet unpersisted config.  Missing
  elements of the ConfigParam are usually ignored until needed (and
  then generally result in errors, but we can't know in advance what
  set of features will be needed).
*/
shared_ptr<Config> cryptar::make_config(const ConfigParam &param)
{
        shared_ptr<Config> config = shared_ptr<Config>(new Config(param));
        // FIXME    The remainder is repeated with the other make_config().
        // FIXME  Fix this actually to look up transport type.  For now we only have one, fs.
        config->m_sender = shared_ptr<Communicator>
                (new Communicator(make_transport(fs_out, shared_ptr<Config>(config))));
        config->m_receiver = shared_ptr<Communicator>
                (new Communicator(make_transport(fs_in, shared_ptr<Config>(config))));
        return config;
}


shared_ptr<Config> cryptar::make_config(const string &in_config_name, const string &in_passphrase)
{
        shared_ptr<Config> config = shared_ptr<Config>(new Config(in_config_name, in_passphrase));
        // FIXME  Fix this actually to look up transport type.  For now we only have one, fs.
        config->m_sender = shared_ptr<Communicator>
                (new Communicator(make_transport(fs_out, shared_ptr<Config>(config))));
        config->m_receiver = shared_ptr<Communicator>
                (new Communicator(make_transport(fs_in, shared_ptr<Config>(config))));
        return config;
}
#endif


shared_ptr<Config> cryptar::make_config(const ConfigParam &param)
{
        return make_shared<Config>(param);
        //return shared_ptr<Config>(new Config(param));
}


/*
  For making new configs from nothing at all.
  Provide what parameters we can in param.

  Missing elements of the ConfigParam are usually ignored until needed
  (and then generally result in errors, but we can't know in advance
  what set of features will be needed).
*/
Config::Config(const ConfigParam &params)
{
        if(!params.m_passphrase.empty())
                m_crypto_key = phrase_to_key(params.m_passphrase);
        m_transport = params.transport();
}



/*
  For making new configs from a stored config.

  Load the named file, decrypt using the passphrase, and rehydrate the
  config.  The file contains the information that we would have
  provided in a ConfigParam instance (below).

  The stored config lives in an encrypted file in the filesystem.
  Config does not enforce policy over where that config file lives:
  in_config_name should be an absolute path unless the client knows it
  wants to do otherwise.

  Changing password is easy if the user wants to change his
  passphrase: we merely need to reread and repersist the config.  But
  if the concern is that the config has been compromised, then we'd
  need to read and repersist everything in the remote store.  (This
  comment should probably move to the change password function once
  it's written, as well as being reproduced in some form in the docs.
  To draw attention to which, I invoke FIXME.)
*/
Config::Config(const string &in_config_name, const std::string &in_passphrase)
{
        if(mode(Verbose))
                cout << "Loading Config(" << in_config_name << ")" << endl;
        if(in_config_name.empty())
                // FIXME:  Also check that file exists
                return;

        m_config_name = in_config_name;
        m_crypto_key = phrase_to_key(in_passphrase);
        ifstream fs(m_config_name, ios_base::binary);
        // FIXME.  I've now repeated this same pattern here and in block::read().
        //         Abstract to a function.
        fs.seekg(0, ios::end);
        int length = fs.tellg();
        fs.seekg(0, ios::beg);
        char *buffer = new char[length];
        fs.read(buffer, length);
        string cipher_text = string(buffer, length);
        fs.close();

        // FIXME    (This is somewhat shared with Block.
        //           Abstract to a function that moves between unencrypted text
        //           and encrypted text in a file.)
        string plain_text = decrypt(cipher_text, m_crypto_key);
        string big_text = decompress(plain_text);
        istringstream big_text_stream(big_text);
        boost::archive::text_iarchive ia(big_text_stream);
        ia & *this;
        // FIXME    (Streaming MUST support m_transport!!!!)
}



/*
  Persist the Config to the named file and with the given passphrase.

  The config is modified to remember the new filename and passphrase.

  The principal reason to permit specifying filename and passphrase is
  to permit copying configs or changing passphrase.  It is also surely
  useful to create new Config's, although this could have also been
  done by insisting on pre-specifying in the initial params.
*/
void Config::save(const string &in_config_name, const string &in_passphrase)
{
        assert(!in_config_name.empty());
        assert(!in_passphrase.empty());

        m_config_name = in_config_name;
        m_crypto_key = phrase_to_key(in_passphrase);
        save();
}



/*
  Persist the Config to the same file as previously and with the same
  passphrase.
*/
void Config::save()
{
        assert(!m_config_name.empty());
        assert(!m_crypto_key.empty());
        
        ostringstream big_text_stream;
        boost::archive::text_oarchive oa(big_text_stream);
        oa & *this;
        string big_text(big_text_stream.str()); // FIXME:  (Is this correct?  What about oa?)
        string plain_text = compress(big_text);
        string cipher_text = encrypt(plain_text, m_crypto_key);

        // FIXME: Abstract this from block::write and so define only once
        ofstream fs(m_config_name, ios_base::binary | ios_base::trunc);
        fs.write(cipher_text.data(), cipher_text.size());
        if(!fs)
                throw_system_error("Config::save()");
        if(mode(Verbose))
                cout << "Config saved." << endl;
}



/*
  Serialize or deserialize according to context.
*/
template<class Archive>
void Config::serialize(Archive &in_ar, const unsigned int in_version)
{
        // FIXME    (Use in_version)
        // FIXME    (Persist transport)
        //in_ar & m_transport;
        in_ar & m_root_id;
        in_ar & m_crypto_key;
}


