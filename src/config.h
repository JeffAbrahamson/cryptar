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



#ifndef __CONFIG_H__
#define __CONFIG_H__ 1


#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <memory>
#include <string>

#include "block.h"


namespace cryptar {

        class Communicator;
        class Config;
        
        
        /* Types for config objects (config.h) */
        // Do not renumber members of this enum.  Values are persisted in config.
        enum TransportType {
                transport_invalid = 0, /* represents an error in type request */
                base_transport,        /* can't be constructed, so an error, but here to be complete */
                no_transport,
                fs,             /* for constructing */
                fs_in,          /* for internal use */
                fs_out          /* for internal use */
                /* and eventually server-based methods (cryptard) */
        };

        struct ConfigParam {
                /*
                  A structure for instantiating new Config's.
                  We document the meaning of the entries in Config,
                  since they are mostly passed through directly.
                */
                std::string m_config_name;
                std::string m_passphrase;
                std::string m_local_dir;
                std::string m_remote_dir;
                std::string m_remote_host;
                TransportType m_transport_type;
        };

        std::shared_ptr<Config> make_config(const ConfigParam &param);
        std::shared_ptr<Config> make_config(const std::string &in_config_name,
                                            const std::string &in_passphrase);
        
        /*
          What to back up.
          Where to put it.
          How to get it there.

          FIXME  Distinguish between a password and a crypto key.  (The former is the user's idea.)
          
          FIXME Who is responsible for transforming passwords
          (passphrases) into crypto keys?  Surely crypt.cpp.
        */
        class Config /*: public Block */{
                friend std::shared_ptr<Config> make_config(const ConfigParam &params);
                friend std::shared_ptr<Config> make_config(const std::string &in_config_name,
                                                           const std::string &in_passphrase);
                
        private:
                /* Make a new config. */
                Config(const struct ConfigParam &param);
                /* Load a config by name. */
                Config(const std::string &in_config_name,
                       const std::string &in_passphrase);

        public:
                // Do I store the key or require it here?
                void save(const std::string in_name = std::string(),
                          const std::string in_passphrase = std::string());

                //// Begin accessors /////////////////////////////////////
                const std::string &local_dir() const { return m_local_dir; };
                void local_dir(const std::string &in) { m_local_dir = in; };

                const std::string &remote_dir() const { return m_remote_dir; };
                void remote_dir(const std::string &in) { m_remote_dir = in; };

                const std::string &remote_host() const { return m_remote_host; };
                void remote_host(const std::string &in) { m_remote_host = in; };

                /* The only reason to set the passphrase is if the
                   user wants to change password.  This may not be the
                   right interface for this, then.  Probably just want
                   a "change_passphrase" function that takes old and
                   new passphrases and returns a bool?
                 */
                //const std::string &crypto_key() const { return m_crypto_key; };
                void crypto_key(const std::string &in) { m_crypto_key = in; };

                /*
                  // I think this should be private, maybe not even stored.
                  // We just need it to create the two comm channels.
                const TransportType &transport_type() const { return m_transport_type; };
                void transport_type(const TransportType &in) { m_transport_type = in; };
                */
                
        private:
                /* We need to be able to create a root block from the
                   config so that when we create a config, we can
                   follow on by creating a root block.  The
                   entanglement comes from the fact that the config
                   must know the id and crypto key of the root block.
                   Letting the client create the root would risk the
                   config not knowing about the root.
                */
                friend class Root;
                const BlockId root_id() const { return m_root_id; };
                void root_id(const BlockId &in_id) { m_root_id = in_id; save(); }
                const std::string &root_block_crypto_key() const;
                
        public:
                //// End accessors ///////////////////////////////////////

                // FIXME   (do I really need both of these here?)
                // FIXME   (perhaps just one comm thread for sending, the other is private)
                std::shared_ptr<Communicator> receiver();
                std::shared_ptr<Communicator> sender();
                
                // FIXME  (are these three needed?)
                std::string staging_dir() const;
                std::string push_to_remote() const;
                std::string pull_from_remote() const;

        private:
                //// Begin persisted data ////////////////////////////////
                // If we store blocks locally, this is where we store them.
                std::string m_local_dir;
                // If we store blocks remotely, this is where they go on the remote host.
                // This may or may not be at the client's discretion, perhaps we'll insist
                // on a server.
                std::string m_remote_dir;
                // If we store blocks remotely, this is the host to which we should connect.
                std::string m_remote_host;

                /* Crypto key for saving the config. */
                /* FIXME: don't leave this as clear text in case of core dump */
                /* FIXME  do we even need to store this? */
                /* The user's pass phrase transforms to a crypto key
                   that lets us decrypt the config file (from which we
                   destreamed a config object).
                */
                std::string m_crypto_key;
                // This is how we will communicate with the remote store.
                enum TransportType m_transport_type;

                // How to find the first block, which points to
                // whatever filesystems and db's there are in the
                // remote store.
                BlockId m_root_id;
                std::string m_root_crypto_key;
                
                //// End persisted data //////////////////////////////////

                std::string m_config_name;
                std::shared_ptr<Communicator> m_receiver;
                std::shared_ptr<Communicator> m_sender;
                
                friend class boost::serialization::access;
                template<class Archive>
                        void serialize(Archive &ar, const unsigned int version);
        };

}

#endif  /* __CONFIG_H__*/
