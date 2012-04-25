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



#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>

#include "block.h"
#include "compress.h"
#include "crypt.h"


using namespace cryptar;
using namespace std;



Block::Block(const CreateEmpty, const string &in_crypto_key)
        : m_crypto_key(in_crypto_key)
{
        m_id = pseudo_random_string();
}


Block::Block(const CreateEmpty, const string &in_crypto_key, string &in_persist_dir)
         : m_crypto_key(in_crypto_key), m_persist_dir(in_persist_dir)
{
        m_id = pseudo_random_string();
}



/*
  Fetch based on block id
*/
Block::Block(const CreateById, const string &in_crypto_key, const BlockId &in_id)
        : m_crypto_key(in_crypto_key), m_id(in_id)
{
}


/*
  Fetch based on block id
*/
Block::Block(const CreateById,
             const string &in_crypto_key,
             const BlockId &in_id,
             string &in_persist_dir)
        : m_crypto_key(in_crypto_key), m_id(in_id), m_persist_dir(in_persist_dir)
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
  Write the cipher text to a file whose name is based on the block id.
  
  It is still an open question whether the flat flag is needed.  It
  surely isn't for the rsync transport.  It might be needed for an
  eventual cloud service provider transport, so keep it for now.  Note
  that flat == true is probably inadequately tested.
*/
void Block::write(const string &in_dir, bool flat) const
{
        string filename = id_to_pathname(in_dir, flat);
        ofstream fs(filename, ios_base::binary | ios_base::trunc);
        fs.write(m_cipher_text.data(), m_cipher_text.size());
        if(!fs) {
                const size_t len = 1024; // arbitrary
                char errstr[len];
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
                strerror_r(errno, errstr, len);
#else
                if(strerror_r(errno, errstr, len))
                        throw("strerror_r() error in Block::write() error");
#endif
                cerr << "Block write error: " << errstr << endl;
                throw("Block::write()");
        }
        fs.close();
}


/*
  Read a staged block into memory.
  Find it by it's id.
*/
void Block::read(const string &in_dir, bool flat)
{
        string filename = id_to_pathname(in_dir, flat);
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
        m_cipher_text = string(buffer, length);
        fs.close();
}


string Block::id_to_pathname(const string &in_dir, bool flat) const
{
        boost::filesystem::path filename;
        string filename_for_id = random_filename(m_id);
        if(flat)
                filename = boost::filesystem::path(in_dir + "-" + filename_for_id);
        else
                filename /= boost::filesystem::path(in_dir) / filename_for_id;
        return filename.string();
}


/*
  Create new based on contents
*/
DataBlock::DataBlock(const CreateById,
                     const string &in_crypto_key,
                     const string &in_id)
        : Block(CreateById(), in_crypto_key, in_id)
{
}


/*
  Create new based on contents
*/
DataBlock::DataBlock(const CreateByContent,
                     const string &in_crypto_key,
                     const string &in_contents)
        : Block(CreateEmpty(), in_crypto_key)
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

