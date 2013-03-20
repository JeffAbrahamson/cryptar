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

        /*
          The RootBlock contains a map of store names to Transport's.

          There is only one RootBlock.  It is not part of RootConfig
          because it is reasonable that the user has multiple hosts
          that should all have the same view of the world.  It is also
          reasonable that multiple users have the same view of the
          world.  Creating a RootConfig pointing to an existing
          RootBlock sets up that view.

          All the Transport's do is tell us how to fetch and store
          blocks for the given store.  A single root (RootBlock) may
          refer to multiple stores, each with different access
          methods.

          The root is a single block (no matter how large it gets, for
          the moment, though we could imagine making it compound if
          need really arises).  It is persisted to a store as defined
          by the Config (cf. config.h), which could be the filesystem
          or could be on the network.  It probably makes more sense
          for the root to be stored on the network, however, since the
          same cryptar data becomes available from all the user's
          cryptar instances if the root is remote.

          The Config tells us how to find the root.

          The root tells us how to find everything else.

          A data store is conceptually a set of (key, value) pairs.
        */
        class RootBlock : public Block {
        public:
                RootBlock(const CreateEmpty,
                          const std::shared_ptr<Transport> in_transport,
                          const std::string &in_crypto_key);
                RootBlock(const CreateByContent,
                          const std::shared_ptr<Transport> in_transport,
                          const std::string &in_crypto_key,
                          const std::string &in_data);
                RootBlock(const CreateById,
                          const std::shared_ptr<Transport> in_transport,
                          const std::string &in_crypto_key,
                          const BlockId &id);
                ~RootBlock();

                const std::shared_ptr<Transport> add_store(const std::string &in_name,
                                                           const ConfigParam &param);
                const std::shared_ptr<Transport> get_store(const std::string &in_name) const;
                void remove_store(const std::string &in_name);
                std::vector<std::string> stores() const;

        private:
                //// Begin persisted data ////////////////////////////////
                std::map<std::string, std::shared_ptr<Transport> > m_stores;
                //// End persisted data //////////////////////////////////
                
                friend class boost::serialization::access;
                template<class Archive>
                        void serialize(Archive &ar, const unsigned int version);
        };

}

#endif  /* __ROOT_H__*/
