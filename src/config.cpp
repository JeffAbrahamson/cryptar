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
  For making new configs from scratch.
*/
Config::Config()
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
*/
Config::Config(const string &in_config_name, const std::string &in_password)
        : m_stage_type(stage_invalid), m_transport_type(transport_invalid)
{
        if(mode(Verbose))
                cout << "Loading Config(" << in_config_name << ")" << endl;
        ifstream fs(in_config_name, ios_base::binary);
        // FIXME.  I've now repeated this same pattern here and in block::read().  Abstract to a function.
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


void Config::save(const string in_config_name, const string in_password)
{
        // Provide both arguments or neither
        assert((in_config_name.empty() && in_password.empty())
               || (!in_config_name.empty() && !in_password.empty()));
        if(!in_config_name.empty() && !in_password.empty()) {
                m_config_name = in_config_name;
                m_password = in_password;
        }

        assert(!m_password.empty());
        assert(!m_config_name.empty());
        
        ostringstream big_text_stream;
        boost::archive::text_oarchive oa(big_text_stream);
        oa & *this;
        string big_text(big_text_stream.str()); // FIXME:  (Is this correct?  What about oa?)
        string plain_text = compress(big_text);
        string cipher_text = encrypt(plain_text, in_password);

        // FIXME: Abstract this from block::write and so define only once
        ofstream fs(in_config_name, ios_base::binary | ios_base::trunc);
        fs.write(cipher_text.data(), cipher_text.size());
        
        if(mode(Verbose))
                cout << "Config saved." << endl;
}


/*
  Return the password with which the config block is encrypted.

  FIXME:
  We need this in order to re-save the config.
  We probably shouldn't need this, and the way to get rid of it is
  immediately to create a root block on creating an empty config.
*/
string Config::password()
{
        // And we shouldn't store this in the clear anyway
        return m_password;
}


/*
  The crypto key for the root block.
  
  Every block stores the cryptographic keys for its children.
*/
string Config::root_block_password()
{
        // FIXME  (Don't store in the clear, at least mask with an xor.)
        // FIXME  (Or maybe better than that.  Or maybe not at all.)
        return m_root_password;
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
        in_ar & m_stage_type;
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
        if(m_receiver)
                return m_receiver;
        m_receiver = shared_ptr<Communicator>(new Communicator(make_stage(m_stage_type, m_local_dir), make_transport(m_transport_type, *this)));
        return m_receiver;
}


/*
  Provide a pointer to the send queue (the object that pushes data to
  the remote store).
*/
shared_ptr<Communicator> Config::sender()
{
        if(m_sender)
                return m_sender;
        m_sender = shared_ptr<Communicator>(new Communicator(make_stage(m_stage_type, m_local_dir), make_transport(m_transport_type, *this)));
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


