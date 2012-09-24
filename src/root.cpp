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
//#include <boost/bind.hpp>
#include <boost/serialization/map.hpp>
#include <boost/thread.hpp>
#include <future>

#include "block.h"
#include "communicate.h"
#include "root.h"


using namespace cryptar;
using namespace std;


/*
  An asynchronous completion token that simply fulfills a promise.
  The point is to provide synchronous requests.

  This class should eventually move to the join the other ACT_*
  classes.
*/
class ACT_Synchronous : public ACT_Base {
public:
        ACT_Synchronous() { m_mutex.lock(); };
        ~ACT_Synchronous() {};
        virtual void operator()() { m_mutex.unlock(); };
        void wait() { m_mutex.lock(); }

private:
        boost::mutex m_mutex;
};

template<class T>
class ACT_Synchronous_Promise : public ACT_Base {
public:
        ACT_Synchronous_Promise(const BlockId &in_id)
                : m_id(in_id) {};
        virtual ~ACT_Synchronous_Promise() {}
        virtual void operator()()
        {
                m_p.set_value(m_id);
        }
        T get_block() { return m_p.get_future().get(); }

private:
        BlockId m_id;
        std::promise<T> m_p;
};

typedef ACT_Synchronous_Promise<Block *> ACT_Block_Synchronous_Promise;
typedef ACT_Synchronous_Promise<BlockId> ACT_BlockID_Synchronous_Promise; // Is this needed?



Root::Root(shared_ptr<Config> in_config)
        : m_config(in_config)
{
        if(!m_config->root_id().empty()) {
                get_from_remote();
                return;
        }
        
        // No root block has ever been persisted, we're starting from scratch.
        // Make and persist an empty root block.  Then update and repersist config.
        
        // FIXME:  (Probably want DirectoryBlock.)
        // FIXME:  (Probably eventually want choice of using head blocks or not.)
        // FIXME:  (Probably even Root should derive from Block and be related to DirectoryBlock.)

        // FIXME:  (Config has created a root block password on instantiation?)
        DirectoryHeadBlock *bp = block_empty<DirectoryHeadBlock>(m_config->root_block_password());
        m_config->root_id(bp->id());
        // Persist the empty root synchronously so that if it fails the config doesn't get repersisted
        // with a root id that is invalid.
        push_to_remote(false);
        m_config->save();
}


Root::~Root()
{
        // FIXME:  Implement
}


/*
  Add the named db to the remote store.
  It is an error to add a db that already exists.
  Return the BlockId of the new db.
*/
BlockId Root::add_db(string &in_name)
{
        assert(m_dbs.find(in_name) == m_dbs.end()); // FIXME:  (Handle db-not-found error more gracefully.)

        string db_crypto_key = pseudo_random_string();
        DirectoryHeadBlock *bp = block_empty<DirectoryHeadBlock>(db_crypto_key);
        m_config->sender()->push(bp);
        
        m_dbs[in_name] = bp->id();
        push_to_remote();
        assert(0);              // Confirm that this function is fully written
        return bp->id();
}


/*
  Return the block pointer of the base of the named db.
*/
BlockId get_db(std::string &in_name)
{
        
        // FIXME:  Implement
        return BlockId();
}


/*
  Fetch the root block from the remote store.
  Populate our name to block id map (m_dbs).
  If this is called again, it acts as a refresh.
*/
void Root::get_from_remote()
{
        // FIXME  (I need a non-fetching create-by-id.)
        // FIXME  (The existing create-by-id functions are explicitly synchronous?)
        DataBlock *bp = block_by_id<DataBlock>(m_config->root_block_password(), m_config->root_id());
        ACT_Synchronous act;
        bp->completion_action(&act);
        m_config->sender()->push(bp);
        act.wait();

        istringstream str(bp->plain_text());
        boost::archive::text_iarchive ia(str);
        ia & *this;
}


/*
  Push the root block to the remote store.

  The root block is the serialized (and encrypted, etc.)
  copy of our name to block id map (m_dbs).

  Asynchronous.
*/
void Root::push_to_remote(const bool in_asynchronous) const
{
        ostringstream big_text_stream;
        boost::archive::text_oarchive oa(big_text_stream);
        oa & *this;
        // FIXME  (Is this correct?)

        // FIXME  (Don't trigger fetch here.)
        DataBlock *bp = block_by_id<DataBlock>(m_config->root_block_password(), m_config->root_id());
        m_config->sender()->push(bp);
}


/*
  Serialize or deserialize according to context.
*/
template<class Archive>
void Root::serialize(Archive &in_ar, const unsigned int in_version)
{
        in_ar & m_dbs;
}

