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

  This owns the pointers.  Don't use the same Stage or Config for
  other purposes.
*/
Communicator::Communicator(const Stage *in_stage, const Config *in_config)
        : m_batch_size(mode(Testing) ? communicator_test_batch_size : communicator_prod_batch_size),
          m_stage(in_stage), m_config(in_config), m_needed(true)
{
        //m_batch_size = mode(Testing) ? communicator_test_batch_size : communicator_prod_batch_size;
        if(mode(Threads))
                run();
}


Communicator::~Communicator()
{
        delete m_stage;
        delete m_config;
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


void Communicator::wait()
{
        while(true) {
                if(queue_empty()) {
                        if(mode(Verbose))
                                cout << "comm: queue is empty, joining..." << endl;
                        m_needed = false;
                        m_thread->join();
                        return;
                }
                if(!m_needed) {
                        if(mode(Verbose))
                                cout << "comm: queue not empty, but thread appears to have terminated."
                                     << endl;
                        return;
                }
                if(mode(Verbose))
                        cout << "comm: waiting for thread to become idle..." << endl;
                sleep(1);
        }
}


/*
  Start a thread.  New threads call operator().
*/
void Communicator::run()
{
        if(mode(Verbose))
                cout << "Starting a new thread." << endl;
        m_thread = new boost::thread(boost::ref(*this));
}


/*
  Run the communication loop until done.
*/
void Communicator::operator()()
{
        if(!mode(Threads)) {
                // If running single-threaded, run once and exit.
                comm_batch();
                return;
        }
        if(mode(Verbose))
                cout << "New thread loop starting." << endl;
        while(m_needed)
                comm_batch();
}


/*
  Run the communication loop once.
*/
void Communicator::comm_batch()
{
        if(m_queue.empty()) {
                // If the queue is empty, sleep a moment to avoid
                // spinning.
                sleep(1);
                return;
        }

        vector<Block *> blocks_to_stage;
        // First gather blocks so that we can release the lock.
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
        // We stage, transport, and then call completion routines
        for_each(blocks_to_stage.begin(),
                 blocks_to_stage.end(),
                 bind1st(mem_fun(&Stage::write), m_stage));
        // ################ transport here ################
        for_each(blocks_to_stage.begin(),
                 blocks_to_stage.end(),
                 boost::bind(&Block::completion_action, _1));
}
