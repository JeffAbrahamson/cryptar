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
#include <string>


namespace cryptar {

        /* Types for config objects (config.h) */
        // Do not renumber members of this enum.  Values are persisted in config.
        enum StageType {
                stage_invalid = 0,
                base_stage,
                stage_out_fs,
                stage_in_fs,
        };

        /* Types for config objects (config.h) */
        // Do not renumber members of this enum.  Values are persisted in config.
        enum TransportType {
                transport_invalid = 0,
                base_transport,
                rsync_push,
                rsync_pull,
        };
        

        /*
          What to back up.
          Where to put it.
          How to get it there.
        */
        class Config {
        public:
                Config();    /* for making new configs */
                Config(const std::string &in_config_name, const std::string &in_password); /* load by name */

                void save(const std::string in_name, const std::string in_password)
                {
                        if(m_config_name.empty())
                                // If we have no name, have a name
                                // If we have a name, keep it (enable Save as...)
                                m_config_name = in_name;
                        if(m_password.empty())
                                m_password = in_password;
                        save_sub(in_name, in_password);
                }

                //// Begin accessors /////////////////////////////////////
                const std::string &local_dir() const { return m_local_dir; };
                void local_dir(const std::string &in) { m_local_dir = in; };

                const std::string &remote_dir() const { return m_remote_dir; };
                void remote_dir(const std::string &in) { m_remote_dir = in; };

                const std::string &remote_host() const { return m_remote_host; };
                void remote_host(const std::string &in) { m_remote_host = in; };

                const std::string &crypto_key() const { return m_crypto_key; };
                void crypto_key(const std::string &in) { m_crypto_key = in; };

                const StageType &stage_type() const { return m_stage_type; };
                void stage_type(const StageType &in) { m_stage_type = in; };

                const TransportType &transport_type() const { return m_transport_type; };
                void transport_type(const TransportType &in) { m_transport_type = in; };
                //// End accessors ///////////////////////////////////////
                
                std::string staging_dir() const;
                std::string push_to_remote() const;
                std::string pull_from_remote() const;

        private:
                //// Begin persisted data ////////////////////////////////
                std::string m_local_dir;
                std::string m_remote_dir;
                std::string m_remote_host;

                /* The user's pass phrase transforms to a crypto key
                   that lets us decrypt the config file (from which we
                   destreamed a config object).  Here in the config
                   object we store the crypto key to be able to read
                   the root object in the remote store.
                */
                std::string m_crypto_key;
                enum StageType m_stage_type;
                enum TransportType m_transport_type;
                
                //// End persisted data //////////////////////////////////

                std::string m_config_name;
                std::string m_password; /* FIXME: don't leave this as clear text in case of core dump */
                void save_sub(const std::string &in_name, const std::string &in_password) const;
                friend class boost::serialization::access;
                template<class Archive>
                        void serialize(Archive &ar, const unsigned int version);
        };

}

#endif  /* __CONFIG_H__*/
