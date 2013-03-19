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


#include <errno.h>
#include <fstream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "block.h"
#include "storage.h"
#include "system.h"


using namespace cryptar;
using namespace std;



FileStorageService::FileStorageService(const string &in_base_path)
        : m_base_path(in_base_path)
{
        // base path, if provided, must end in '/'
        if(!m_base_path.empty())
                //assert('/' == *(m_base_path.end()--)); // FIXME    (OS-specific)
                assert('/' == m_base_path.back()); // FIXME    (OS-specific)
        if(mkdir(in_base_path.c_str(), 0700))
                if(EEXIST != errno)
                        throw_system_error("FileStorageService::FileStorageService()");
}


FileStorageService::~FileStorageService()
{
}


/*
  Return the name of the file in which this block's content should be
  stored.
*/
const string FileStorageService::block_to_filename(const Block *in_block) const
{
        return m_base_path + message_digest(in_block->id().as_string(), true);
}


/*
  Write this block's contents to a file.
*/
void FileStorageService::write(const Block *in_block)
{
        const string filename(block_to_filename(in_block));
        const string payload(in_block->to_stream());
        ofstream fs(filename, ios_base::binary | ios_base::trunc);
        fs.write(payload.data(), payload.size());
        if(!fs)
                throw_system_error("FileStorageService::write()");
        fs.close();
}


/*
  Read and set this block's contents from a file.
*/
void FileStorageService::read(Block *in_block)
{
        string filename(block_to_filename(in_block));
        ifstream fs(filename, ios_base::binary);
        if(!fs) {
                const size_t len = 1024; // arbitrary
                char errstr[len];
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
                perror(0);
                cout << "(just called perror(0))" << "  errno=" << errno << endl;
                cout << "filename=" << filename << endl;
                if(strerror_r(errno, errstr, len))
                        throw("strerror_r() error in Block::read() error");
#else
                strerror_r(errno, errstr, len);
#endif
                cerr << "Block read error: " << errstr << endl;
                throw("Block::read()");
        }
        fs.seekg(0, ios::end);
        int length = fs.tellg();
        fs.seekg(0, ios::beg);
        char *buffer = new char[length];
        fs.read(buffer, length);
        string payload(buffer, length);
        in_block->from_stream(payload);
        fs.close();
}


/*
  Remove the file that persisted this block's contents.
  Does not modify the block itself.

  FIXME    (Perhaps we should mark the block in need of persisting?)
*/
void FileStorageService::remove(const Block *in_block) const
{
        string filename(block_to_filename(in_block));
        if(::remove(filename.c_str()))
                // FIXME    (Better error handling, and report file name)
                perror("Error deleting file");
}

