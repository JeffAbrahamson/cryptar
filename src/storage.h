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


class Block;


// FIXME:  StorageService probably needs to know about a Communicator
class StorageService {
 public:
        StorageService();
        virtual ~StorageService();

        // Call to_stream on the block and write the result somewhere.
        virtual void write(const std::shared_ptr<Block> in_block) = 0;
        // Fetch the block's contents from somewhere, then call
        // from_stream on the block.
        virtual void read(std::shared_ptr<Block> in_block) = 0;
};


class FileStorageService {
 public:
        FileStorageService();
        virtual ~FileStorageService();

        // Write the serialized block to a file
        virtual void write(const std::shared_ptr<Block> in_block);
        // Read the serialized block from a file
        virtual void read(std::shared_ptr<Block> in_block);
};


// FIXME    Implement this
class NetStorageService {
 public:
        NetStorageService();
        virtual ~NetStorageService();

        // Schedule the serialized block for transfer to a remote store
        virtual void write(const std::shared_ptr<Block> in_block);
        // Request the block be pulled from a remote store
        virtual void read(std::shared_ptr<Block> in_block);

        // To find out if a transfer worked, call the block's ready() method.
        // FIXME    or something like that
};


#endif  /* __STORAGE_H__*/
