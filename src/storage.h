/*
  Copyright 2013  Jeff Abrahamson
  
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



#ifndef __STORAGE_H__
#define __STORAGE_H__ 1


#include <memory>
#include <string>


namespace cryptar {

        class Block;


        // FIXME:  StorageService probably needs to know about a Communicator
        class StorageService {
        public:
                StorageService();
                virtual ~StorageService();

                // Call to_stream on the block and write the result somewhere.
                virtual void write(const Block *in_block) = 0;
                
                // Fetch the block's contents from somewhere, then call
                // from_stream on the block.
                virtual void read(Block *in_block) = 0;

                // Remove block from store.
                virtual void remove(const Block *in_block) const = 0;
        };


        /*
          Marshall between blocks and their persisted state on disk.
          
          FileStorageService uses the block id as filename.
          
          If, someday, we need to be fancier, which is quiet likely,
          we should probably split into directories based on the first
          couple characters of the block id.  It is even possible to
          rebalance and/or to try multiple names (abcdefgh, then
          ab/cdefgh) if we need to.

          It would be nice to associate the block id with a sequence
          number, but we don't want to give away too much information
          about when blocks were created (although file creation time
          is an awfully good hint, but it's a hint that gets obscured
          as files are re-written).

          The other problem with sequence id's is managing them
          between different instances of the program and even usage
          from different hosts.
        */
        class FileStorageService {
        public:
                FileStorageService(const std::string &in_base_path);
                virtual ~FileStorageService();

                // Write the serialized block to a file
                virtual void write(const Block *in_block);
                // Read the serialized block from a file
                virtual void read(Block *in_block);
                // Remove the file from the store
                virtual void remove(const Block *in_block) const;

        private:
                const std::string block_to_filename(const Block *in_block) const;
                
                std::string m_base_path;
        };


        // FIXME    Implement this
        /*
          Marshall between blocks and their persisted state on the network.
          
          Where FileStorage Service stores to a file,
          NetStorageService pushes to a communication stream.
        */
        class NetStorageService {
        public:
                NetStorageService();
                virtual ~NetStorageService();

                // Schedule the serialized block for transfer to a remote store
                virtual void write(const Block *in_block);
                // Request the block be pulled from a remote store
                virtual void read(Block *in_block);
                // Remove the file from the store
                virtual void remove(const Block *in_block) const;

                // To find out if a transfer worked, call the block's ready() method.
                // FIXME    or something like that
        };

}

#endif  /* __STORAGE_H__*/
