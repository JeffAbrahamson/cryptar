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
#include <boost/serialization/string.hpp>
#include <fstream>
#include <sstream>
#include <string>

#include "communicate.h"
#include "compress.h"
#include "config.h"
#include "crypt.h"
#include "mode.h"


using namespace cryptar;
using namespace std;


/*
  Cf. comments at Config::Config below.
*/
shared_ptr<Config> cryptar::make_config(const string &in_passphrase)
{
        // FIXME  Needs a transport type.
        return make_config("", in_passphrase);
}


shared_ptr<Config> cryptar::make_config(const string &in_config_name, const string &in_passphrase)
{
        shared_ptr<Config> config = shared_ptr<Config>(new Config(in_config_name, in_passphrase));
        // FIXME  Fix this actually to look up transport type.  For now we only have one.
        config->m_sender = shared_ptr<Communicator>
                (new Communicator(make_transport(fs_out, shared_ptr<Config>(config))));
        config->m_receiver = shared_ptr<Communicator>
                (new Communicator(make_transport(fs_in, shared_ptr<Config>(config))));
        return config;
}



/*
  For making new configs from scratch.
  This should only be called by make_config(), above.
*/
Config::Config(const string &in_passphrase)
{
}


/*
  For making new configs from a stored config.  The stored config
  lives in an encrypted file on the local filesystem.  Config does not
  enforce policy over where that config file lives: in_config_name
  should be an absolute path unless the client knows it wants to do
  otherwise.

  Note that in_password is not the user's passphrase, but the hashed
  key we derive from it.  The config, in turn, stores the crypto key
  for accessing remote content.

  Changing password is easy if the user wants to change his
  passphrase: we merely need to reread and repersist the config.  But
  if the concern is that the config has been compromised, then we'd
  need to read and repersist everything in the remote store.  (This
  comment should probably move to the change password function once
  it's written, as well as being reproduced in some form in the docs.
  To draw attention to which, I invoke FIXME.)
  
  This should only be called by make_config(), above.
*/
Config::Config(const string &in_config_name, const std::string &in_password)
        : m_transport_type(transport_invalid)
{
        if(mode(Verbose))
                cout << "Loading Config(" << in_config_name << ")" << endl;
        if(in_config_name.empty())
                // FIXME:  Also check that file exists
                return;

        ifstream fs(in_config_name, ios_base::binary);
        // FIXME.  I've now repeated this same pattern here and in block::read().
        //         Abstract to a function.
        fs.seekg(0, ios::end);
        int length = fs.tellg();
        fs.seekg(0, ios::beg);
        char *buffer = new char[length];
        fs.read(buffer, length);
        string cipher_text = string(buffer, length);
        fs.close();

        string plain_text = decrypt(cipher_text, in_password);
        string big_text = decompress(plain_text);
        istringstream big_text_stream(big_text);
        boost::archive::text_iarchive ia(big_text_stream);
        ia & *this;
}


void Config::save(const string in_config_name, const string in_passphrase)
{
        // Provide both arguments or neither
        assert((in_config_name.empty() && in_passphrase.empty())
               || (!in_config_name.empty() && !in_passphrase.empty()));
        if(!in_config_name.empty() && !in_passphrase.empty()) {
                m_config_name = in_config_name;
                m_crypto_key = phrase_to_key(in_passphrase);
        }

        assert(!m_crypto_key.empty());
        assert(!m_config_name.empty());
        
        ostringstream big_text_stream;
        boost::archive::text_oarchive oa(big_text_stream);
        oa & *this;
        string big_text(big_text_stream.str()); // FIXME:  (Is this correct?  What about oa?)
        string plain_text = compress(big_text);
        string cipher_text = encrypt(plain_text, m_crypto_key);

        // FIXME: Abstract this from block::write and so define only once
        ofstream fs(in_config_name, ios_base::binary | ios_base::trunc);
        fs.write(cipher_text.data(), cipher_text.size());
        
        if(mode(Verbose))
                cout << "Config saved." << endl;
}


/*
  The crypto key for the root block.
  
  Every block stores the cryptographic keys for its children.
  FIXME  Should we instead return a future for the block?
*/
const string &Config::root_block_crypto_key() const
{
        // FIXME  (Don't store in the clear, at least mask with an xor.)
        // FIXME  (Or maybe better than that.  Or maybe not at all.)
        return m_root_crypto_key;
}



/*
  Serialize or deserialize according to context.
*/
template<class Archive>
void Config::serialize(Archive &in_ar, const unsigned int in_version)
{
        //////////////// FIXME: use in_version
        in_ar & m_local_dir;
        in_ar & m_remote_dir;
        in_ar & m_remote_host;
        in_ar & m_crypto_key;
        in_ar & m_transport_type;
        in_ar & m_root_id;
}



string Config::staging_dir() const
{
        return string("/tmp/cryptar-") + getenv("HOME");
}


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


string Config::push_to_remote() const
{
        //////////////// implement this  FIXME
        return string();
}


string Config::pull_from_remote() const
{
        //////////////// implement this  FIXME
        return string();
}


