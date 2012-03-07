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


Block::Block()
{
}



/*
  Create new based on contents
*/
Block::Block(const std::string &contents)
{
}


/*
  Fetch based on block id
*/
Block::Block(const BlockId id)
{
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




