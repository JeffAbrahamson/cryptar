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


#include <string>


namespace cryptar {

        /*
          What to back up.
          Where to put it.
          How to get it there.
        */
        class Config {
        public:
                /*
                  How will this work?  Probably in real life, we'll
                  either create a new instance and populate from user
                  input or else create it from a name, which means
                  reading an (encrypted) file and populating the
                  fields that way.

                  How to represent and then compute the transfer and
                  staging info?
                */
                Config();    /* for making new configs */
                Config(const std::string &config_name);

                const std::string &local_dir() const { return m_local_dir; };
                void local_dir(const std::string &in) { m_local_dir = in; };

                const std::string &remote_dir() const { return m_remote_dir; };
                void remote_dir(const std::string &in) { m_remote_dir = in; };

                const std::string &remote_host() const { return m_remote_host; };
                void remote_host(const std::string &in) { m_remote_host = in; };

                std::string staging_dir() const;
                std::string push_to_remote() const;
                std::string pull_from_remote() const;

        private:
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
        };

}

#endif  /* __CONFIG_H__*/
