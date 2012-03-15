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



#include "cryptar.h"


using namespace cryptar;
using namespace std;

Block::Block(const string &in_crypto_key)
         : m_crypto_key(in_crypto_key)
{
        m_id = pseudo_random_string();
}



/*
  Fetch based on block id
*/
Block::Block(const string &in_crypto_key, const BlockId in_id)
        : m_crypto_key(in_crypto_key), m_id(in_id)
{
}


Block::~Block()
{
        // Clean up ACT queue if it exists (and warn of error, as this
        // shouldn't happen).
        if(m_act_queue.empty())
                return;
        cerr << "ACT queue contains " << m_act_queue.size() << " objects at deletion.  (Expected zero.)" << endl;
        while(!m_act_queue.empty()) {
                // Clean them up anyway
                ACT_Base *act = m_act_queue.front();
                delete act;
                m_act_queue.pop();
        }
}


/*
  Add an asynchronous completion token (ACT).

  This is thread-safe, as only one thread at a time has control over a
  given block.  So that thread may execute ACT's from that block and
  may modify the ACT queue.

  If an ACT modifies another block, it must handle thread saftey for
  that action.

  The ACT is deleted after being executed.
*/
void Block::completion_action(ACT_Base *act)
{
        m_act_queue.push(act);
}


/*
  Do the completion actions in the order that they were pushed.
  Clear the queue.
*/
void Block::completion_action()
{
        while(!m_act_queue.empty()) {
                ACT_Base *act = m_act_queue.front();
                (*act)();
                delete act;
                m_act_queue.pop();
        }
}


/*
  Create new based on contents
*/
DataBlock::DataBlock(const string &in_crypto_key, const string &in_contents)
        : Block(in_crypto_key)
{
        string augmented_content = pseudo_random_string(11) + in_contents;
        m_cipher_text = encrypt(compress(augmented_content), m_crypto_key);
}


/*
  Return plain text of block.
*/
string DataBlock::plain_text() const
{
        string augmented_text = decompress(decrypt(m_cipher_text, m_crypto_key));
        return augmented_text.substr(11);
}

