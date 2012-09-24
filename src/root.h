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



#ifndef __ROOT_H__
#define __ROOT_H__ 1


#include <map>
#include <memory>
#include <string>

#include "block.h"
#include "config.h"


namespace cryptar {

        class Root {
        public:
                Root(std::shared_ptr<Config> in_config);
                ~Root();

                BlockId add_db(std::string &in_name);
                BlockId get_db(std::string &in_name);

                // Maybe some day offer to remove db's.
                // Maybe some day offer to query what db's there are.

        private:
                void get_from_remote();
                void push_to_remote(const bool in_asynchronous = true) const;
                
                std::shared_ptr<Config> m_config;
                /* m_dbs is a map of db names to the block id of the root of the named db. */
                /* FIXME  (Actually needs to be map of name to (block id of db, crypto key of db) */
                std::map<std::string, BlockId> m_dbs;
                
                friend class boost::serialization::access;
                template<class Archive>
                        void serialize(Archive &ar, const unsigned int version);
        };

}

#endif  /* __ROOT_H__*/
