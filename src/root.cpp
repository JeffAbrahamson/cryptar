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
#include "config.h"
#include "communicate.h"
#include "root.h"


using namespace cryptar;
using namespace std;


/*
  An asynchronous completion token that simply fulfills a promise.
  The point is to provide synchronous requests.

  This class should eventually move to the join the other ACT_*
  classes.  Or maybe it should become part of blocks.
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


/*
  Create new and empty.
*/
RootBlock::RootBlock(const CreateEmpty,
                     const shared_ptr<Transport> in_transport,
                     const string &in_crypto_key)
        : Block(CreateEmpty(), in_transport, in_crypto_key)
{
}


/*
  Create new based on contents
*/
RootBlock::RootBlock(const CreateByContent,
                     const shared_ptr<Transport> in_transport,
                     const string &in_crypto_key,
                     const string &in_contents)
        : Block(CreateEmpty(), in_transport, in_crypto_key)
{
        // FIXME    (This may not be needed.  Delete or implement.)
}


/*
  Create new based on ID.
*/
RootBlock::RootBlock(const CreateById,
                     const shared_ptr<Transport> in_transport,
                     const string &in_crypto_key,
                     const BlockId &in_id)
        : Block(CreateById(), in_transport, in_crypto_key, in_id)
{
        // FIXME    (Implement)

        /*
          Never had a RootBlock, so this is new.
          Make the block.
          Then persist it (call save(.,.)).
        */
}


RootBlock::~RootBlock()
{
        // FIXME    (Implement if needed.  Document.)
}


/*
  Add the named store.
  It is an error to add a store name that already exists.
  Return a Transport for the new store.
*/
const shared_ptr<Transport> RootBlock::add_store(const string &in_name,
                                                 const ConfigParam &param)
{
        // FIXME    (Handle not-found error more gracefully.)
        assert(m_stores.find(in_name) == m_stores.end());
        m_stores[in_name] = param.transport();
        return param.transport();
}


/*
  Return a transport to the named store.
*/
const shared_ptr<Transport> RootBlock::get_store(const string &in_name) const
{
        auto it = m_stores.find(in_name);
        if(m_stores.end() == it)
                assert(0);      // FIXME    (Handle this error)
        return it->second;
}


void RootBlock::remove_store(const string &in_name)
{
        auto it = m_stores.find(in_name);
        if(m_stores.end() == it)
                assert(0);      // FIXME    (Handle this error)
        m_stores.erase(it);
}



vector<string> RootBlock::stores() const
{
        vector<string> keys;
        for(auto element : m_stores)
                keys.push_back(element.first);
        return keys;
}



#if 0
/*
  Fetch the root block from the remote store.
  Populate our name to block id map (m_dbs).
  If this is called again, it acts as a refresh.
*/
void RootBlock::get_from_remote()
{
        // FIXME  (I need a non-fetching create-by-id.)
        // FIXME  (The existing create-by-id functions are explicitly synchronous?)
        DataBlock *bp = block_by_id<DataBlock>(m_config->root_block_crypto_key(), m_config->root_id());
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
void RootBlock::push_to_remote(const bool in_asynchronous) const
{
        ostringstream big_text_stream;
        boost::archive::text_oarchive oa(big_text_stream);
        oa & *this;
        // FIXME  (Is this correct?)

        // FIXME  (Don't trigger fetch here.)
        DataBlock *bp = block_by_id<DataBlock>(m_config->root_block_crypto_key(), m_config->root_id());
        m_config->sender()->push(bp);
}
#endif


/*
  Serialize or deserialize according to context.
*/
template<class Archive>
void RootBlock::serialize(Archive &in_ar, const unsigned int in_version)
{
        in_ar & m_stores;
}

