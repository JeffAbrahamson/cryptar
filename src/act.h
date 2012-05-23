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



#ifndef __ACT_H__
#define __ACT_H__ 1


#include "block.h"
#include "filesystem.h"


namespace cryptar {

        /*
          Asynchronous Completion Tokens.

          We might say "object" rather than "token" but for usage by
          Douglas C. Schmidt, 1998, 1999:
          http://www.cs.wustl.edu/~schmidt/PDF/ACT.pdf

          The definition of the abstract base class ACT_Base lives in
          block.h, since it must be defined there for blocks to compile.
        */

        /*
          ACT that selects a Head object from a Timeline and queues
          fetching it.
        */
        class Timeline_HeadSelector : public ACT_Base {
                // Select most recent Head less than or equal to time.
                // If time is zero, select the most recent Head.
                Timeline_HeadSelector(TimelineBlock *tlb, time_t time);
                ~Timeline_HeadSelector();

                virtual void operator()();
        };


        /*
          ACT that fetches the blocks of a head object.
          If levels > 0, recursively fetch sub-objects.
          If the Head object does not represent a directory,
          levels has no effect.
        */
        class Head_FetchBlocks : public ACT_Base  {
                Head_FetchBlocks(/*Head*/Block *hb, int levels = 0);
                ~Head_FetchBlocks();

                virtual void operator()();
        };


        /*
          ACT that inserts a file into a filesystem object.
        */
        class Head_InsertFS : public ACT_Base  {
                Head_InsertFS(/*Head*/Block *hb, FS_Node *fsn);
                ~Head_InsertFS();

                virtual void operator()();
        };


        /*
          ACT to note that a block has been fetched.
        */
        class Block_NoteFetched : public ACT_Base  {
                Block_NoteFetched(Block *b);
                ~Block_NoteFetched();

                virtual void operator()();
        };


        /*
          ACT to call a trigger on a Head or Timeline object.
          Calls other->act().
          For example, the head object can do something when all of
          its blocks have been fetched via

              act() {
                  if(count++ == m_num_blocks)
                      do_something();
              }
        */
        class Block_Trigger : public ACT_Base  {
                Block_Trigger(Block *b, Block *other);
                ~Block_Trigger();

                virtual void operator()();
        };
}


#endif  /* __ACT_H__*/
