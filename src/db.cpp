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


#include "db.h"


using namespace cryptar;
using namespace std;


template<typename T>
DBSet<T>::DBSet(shared_ptr<Config> in_config, string in_dbname)
{
        // The config provides us with access to communication
        // channels (Communicator instances).
        m_config = in_config;
        
        // Find db from root, keep a pointer to the root's head block
        

        // Set whatever other local state we need, including a ready flag
        
}



template<typename T>
DBSet<T>::~DBSet()
{
        // Unset ready flag
        
        // Request communicator shutdown
        
}



/*
  Return a T given its unique identifier.

  Fetches the block(s) from the remote store, decrypts, and constructs
  the T.

  FIXME: Support composite objects.  For now, we're only fetching
  single blocks, but we should also support assembly of more complex
  objects, including cryptar algorithm support.
*/
template<typename T>
T DBSet<T>::get(const BlockId &in_id)
{
        // Tell communicator we want block.  ACT fulfills promise.

        // Wait on promise.
}


/*
  Insert a new T.

  We don't check if an identical T already exists.
*/
template<typename T>
void DBSet<T>::put(const T &in_T)
{
        // Send block

        // Wait on promise

        // Probably just want to assign an id, then call put with id.
}


/*
  Update an existing T.

  If the block ID doesn't exist, it will be created, but that's a bit
  dangerous.  FIXME:  Check to make sure block id's are valid.
*/
template<typename T>
void DBSet<T>::put(const BlockId &in_id, const T &in_T)
{
        // Send block

        // Wait on promise
        
}

