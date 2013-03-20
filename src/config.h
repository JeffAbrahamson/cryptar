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

        class Config;
        class RootBlock;
        class Transport;
        
        /* Types for config objects (config.h) */
        // Do not renumber members of this enum.  Values are persisted in config.
        enum TransportType {
                invalid_transport = 0, /* represents an error in type request */
                base_transport,        /* can't be constructed, so an error, but here to be complete */
                no_transport,
                fs,             /* storage in filesystem */
                /* and eventually server-based methods (cryptard) */
        };

        class ConfigParam {
                /*
                  A structure for instantiating new Config's.
                  We document the meaning of the entries in Config,
                  since they are mostly passed through directly.
                */
        public:
                ConfigParam(TransportType in_transport)
                        : m_transport_type(in_transport) {
                        assert(invalid_transport != m_transport_type);
                }
                std::string m_config_name;
                std::string m_passphrase;
                std::string m_local_dir;
                std::string m_remote_dir;
                std::string m_remote_host;
                TransportType m_transport_type;

                const std::shared_ptr<Transport> transport() const;

        private:
                const std::shared_ptr<Transport> make_transport() const;

                mutable std::shared_ptr<Transport> m_transport;
        };

#if 0
        std::shared_ptr<Config> make_config(const ConfigParam &param);
        std::shared_ptr<Config> make_config(const std::string &in_config_name,
                                            const std::string &in_passphrase);
#endif

        /*
          The RootConfig tells us how to find (or store) the RootBlock.

          There is only one RootConfig.  It should be stored locally,
          generally this means in the filesystem.  We can create a new
          one, persist (or re-persist) it, and instantiate an existing
          one from its file.

          It's only useful function is root(), which synchronously
          fetches and returns the root block (or creates a new root
          block and returns that).  If it creates a new root block, it
          persists it before returning it.

          The root block may exist in the file system or on the net.

          Cf. root.h / root.cpp for more about RootBlock's.
        */

        /*
          FIXME   (Distinguish between a password and a crypto key.
                   The former is the user's idea.)
          
          FIXME   (Who is responsible for transforming passwords (passphrases) into crypto keys?
                   Surely crypt.cpp.)
        */
        class Config {
                /*
                friend std::shared_ptr<Config> make_config(const ConfigParam &params);
                friend std::shared_ptr<Config> make_config(const std::string &in_config_name,
                                                           const std::string &in_passphrase);
                */
        public:
                /* Make a new config. */
                Config(const ConfigParam &param);
                /* Load a config by name. */
                Config(const std::string &in_config_name,
                       const std::string &in_passphrase);

                // Persist the Config to a file.
                void save();
                void save(const std::string &in_name,
                          const std::string &in_passphrase);
                // Fetch the RootBlock (or create) and return.
                std::shared_ptr<RootBlock> root();

        private:
                /* We need to be able to create a root block from the
                   config so that when we create a config, we can
                   follow on by creating a root block.  The
                   entanglement comes from the fact that the config
                   must know the id and crypto key of the root block.
                   Letting the client create the root would risk the
                   config not knowing about the root.
                */
                const BlockId root_id() const { return m_root_id; };
                void root_id(const BlockId &in_id) { m_root_id = in_id; save(); }
                
        private:
                //// Begin persisted data ////////////////////////////////
                
                // This is how we will communicate with the store.
                std::shared_ptr<Transport> m_transport;

                // How to find the RootBlock (cf. root.h)
                BlockId m_root_id;
                std::string m_root_crypto_key;
                
                //// End persisted data //////////////////////////////////

                std::string m_config_name;
                /* Crypto key for saving the config.

                   The user's pass phrase transforms to a crypto key
                   that lets us decrypt the config file (from which we
                   destreamed a config object).
                */
                std::string m_crypto_key;

                friend class boost::serialization::access;
                template<class Archive>
                        void serialize(Archive &ar, const unsigned int version);
        };

        std::shared_ptr<Config> make_config(const ConfigParam &param);
}

#endif  /* __CONFIG_H__*/
