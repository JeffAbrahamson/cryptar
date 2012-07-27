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



#ifndef __DB_H__
#define __DB_H__ 1

namespace cryptar {

        Template<T>
        class DBSet {
                /*
                  A DBSet presents a simple blob store.  On storing an
                  object, the client is returned a unique blob
                  identifier that is later used to retrieve the
                  object.

                  The template parameter T is the thing that is
                  stored.  It should know how to serialize and
                  unserialize itself.  (FIXME: Note interface.)
                  T needn't (indeed, generally shouldn't) know
                  anything else about cryptar or its block structure.
                */

                /*
                  A note on the structure of cryptar stores:

                  The local store provides only enough data to contact
                  the local store and to request a root object.  All
                  else is remote.  The root object is provides a
                  directory of remote store names and pointers to
                  their head blocks.  When the user requests a handle
                  to a remote store, we look in the root store and
                  construct an appropriate handle.  The remote store
                  types are likely DBSet, DBMap, and FileSystem.

                  FIXME:  Check this comment for consistency of
                  terminology.  Also move to some more logical place
                  (config.h?) and include here a pointer to the
                  comment.
                */

                /* FIXME:  Probably constructor should be a friend of
                   the class that knows how to make these. */
        public:
                DBSet();
                ~DBSet();

                /*
                  Return a T given its unique identifier.  Fetches
                  the block(s) from the remote store, decrypts, and
                  constructs the T.
                  
                  FIXME:  Don't reveal BlockID here, make a new type.
                  FIXME:  Use move semantics.
                */
                T get(const BlockId &in_id);
                /* Insert a new T. */
                put(const T &in_T);
                /* Update an existing T. */
                put(const BlockId &in_id, const T &in_T);

        private:
                /* Config?  Other? */
        };

        Template<T>
        class DBMap {
                /* A DBMap presents the user with a key - value store.
                   The implementation, however, is a wrapper around a
                   DBSet with an index mapping between blob id's and
                   keys.
                */
        public:
                DBMap();
                ~DBMap();

        private:
                DBSet *m_dbset;
        };

}

#endif  /* __DB_H__*/
