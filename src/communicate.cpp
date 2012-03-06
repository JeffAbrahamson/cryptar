/*
  Copyright 2012  Jeff Abrahamson
  
  This file is part of cryptar.
  
  This software might be free, it might be commercial.
  You may safely assume that using it for personal, non-commercial
  use will remain acceptable.  The eventual license may, however, be a
  split free/commercial license, such as the one long used by
  Sleepycat.

  In any case, this software is explicitly licensed the terms of the
  GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any
  later version.
  
  cryptar is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with cryptar.  If not, see <http://www.gnu.org/licenses/>.
*/



#include <boost/thread.hpp>
#include <cassert>

#include "cryptar.h"


using namespace cryptar;
using namespace std;


namespace {

}


/*

  Eventually set m_batch_size to 500 or something.  Tune to a value
  that makes sense.  Provide auto-tuning function so others can tune
  to their environments.
*/
Communicator::Communicator() : m_needed(false), m_batch_size(1), m_stage(new Stage())
{
        if(!mode(Testing))
                run();
}


Communicator::~Communicator()
{
        delete m_stage;         // ################
}


/*
  Push a block on the queue of blocks to communicate (transfer between
  local and remote host).

  track count locally?

*/
void Communicator::push(Block *bp)
{
        boost::lock_guard<boost::mutex> lock(m_queue_access);
        m_queue.push(bp);
}


bool Communicator::queue_empty()
{
        boost::lock_guard<boost::mutex> lock(m_queue_access);
        return m_queue.empty();
}


/*
  Function needed?
*/
Block *Communicator::pop()
{
        boost::lock_guard<boost::mutex> lock(m_queue_access);
        cerr << "Protect against empty pop() here." << endl; // ################
        Block *bp = m_queue.front();
        m_queue.pop();
        return bp;
}


/*
  Start a thread that calls comm_loop().
*/
void Communicator::run()
{
        m_thread = new boost::thread(boost::ref(*this));
}


/*
  Run the communication loop until done.
*/
void Communicator::operator()()
{
        while(m_needed)
                comm_batch();
}


/*
  Run the communication loop once.
*/
void Communicator::comm_batch()
{
        if(m_queue.empty()) {
                sleep(1);
                return;
        }

        vector<Block *> blocks_to_stage;
        {
                boost::lock_guard<boost::mutex> lock(m_queue_access);
                if(m_queue.empty())
                        return;
                queue_size_type num_blocks_to_stage = min(m_queue.size(), m_batch_size);
                blocks_to_stage.reserve(num_blocks_to_stage);
                for(queue_size_type i = 0; i < num_blocks_to_stage; i++) {
                        blocks_to_stage.push_back(m_queue.front());
                        m_queue.pop();
                }
        }
        for_each(blocks_to_stage.begin(),
                 blocks_to_stage.end(),
                 //bind1st(mem_fun1_t<void, Stage, Block *>(&Stage::write), m_stage));
                 bind1st(mem_fun(&Stage::write), &m_stage));

}
