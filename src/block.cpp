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


/*
  On Playing with Blocks

  Blocks represent data that (potentially) exists in both the local
  and remote stores.  Block id's identify the block in the remote
  store as well as in the locally running process.

  Instantiating a block by id causes it to be fetched from the local
  store.  The flag m_status flag is BlockStatus::Invalid until the
  fetch completes.  On successful fetch, m_status ==
  BlockStatus::Ready, if the block is not found m_status ==
  BlockStatus::NotFound.

  Instantiating a block by content causes a new id to be assigned and
  the block to be pushed to the remote store.

  To update a DataBlock, instantiate by id, then call SetContent().
*/



/******************************************************************************/
/* BlockId */


// Defined inline in block.h


/******************************************************************************/
/* Block */

Block::Block(const CreateEmpty, const string &in_crypto_key)
        : m_crypto_key(in_crypto_key),
          m_status(BlockStatus::ready | BlockStatus::dirty)
{
        m_id = pseudo_random_string();
}


Block::Block(const CreateEmpty, const string &in_crypto_key, string &in_persist_dir)
        : m_crypto_key(in_crypto_key),
          m_status(BlockStatus::ready | BlockStatus::dirty),
          m_persist_dir(in_persist_dir)
{
        m_id = pseudo_random_string();
}



/*
  Fetch based on block id
*/
Block::Block(const CreateById, const string &in_crypto_key, const BlockId &in_id)
        : m_crypto_key(in_crypto_key), m_id(in_id), m_status(BlockStatus::block_status_invalid)
{
        // Here trigger fetch from remote
}


/*
  Fetch based on block id
*/
Block::Block(const CreateById,
             const string &in_crypto_key,
             const BlockId &in_id,
             string &in_persist_dir)
        : m_crypto_key(in_crypto_key),
          m_id(in_id),
          m_status(BlockStatus::block_status_invalid),
          m_persist_dir(in_persist_dir)
{
        // Here trigger fetch from remote
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
        string filename_for_id = filename_from_random_bits(m_id.as_string());
        if(flat)
                filename = boost::filesystem::path(in_dir + "-" + filename_for_id);
        else
                filename /= boost::filesystem::path(in_dir) / filename_for_id;
        return filename.string();
}


/******************************************************************************/
/* DataBlock */

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
  Create new based on ID.
  
  Interpretting the encrypted data is left to those who derive from
  DataBlock.  We have no idea what structure the data has, we can only
  provide the plain text on request.
*/
DataBlock::DataBlock(const CreateById,
                     const string &in_crypto_key,
                     const BlockId &in_id)
        : Block(CreateById(), in_crypto_key, in_id)
{
}


/*
  Return plain text of block.

  We don't store it.  The fewer copies of plain text we have floating
  about, the better.
*/
string DataBlock::plain_text() const
{
        string augmented_text = decompress(decrypt(m_cipher_text, m_crypto_key));
        return augmented_text.substr(11);
}


/*
  Set the DataBlock's contents (by providing plain text).
*/
void DataBlock::set_content(const string &in_contents)
{
        string augmented_content = pseudo_random_string(11) + in_contents;
        m_cipher_text = encrypt(compress(augmented_content), m_crypto_key);
}


/******************************************************************************/
/* CoverBlock */


static const unsigned int m_window_size = 512;


/*
  Create new based on contents
*/
CoverBlock::CoverBlock(const CreateByContent,
                     const string &in_crypto_key,
                     const string &in_contents)
        : DataBlock(CreateByContent(), in_crypto_key, in_contents),
          m_base(0), m_base_length(0)
{
        // Steps for creating by contents:
        // Compute a cover
        // Create a set of DataBlock's for the cover
        // Store pointers to the DataBlocks in m_data_blocks
        // Persist the data blocks in remote store
        // Populate m_easy_checksums and m_crypto_checksums
}


/*
  Create new based on ID
*/
CoverBlock::CoverBlock(const CreateById,
                       const string &in_crypto_key,
                       const BlockId &in_id)
        : DataBlock(CreateById(), in_crypto_key, in_id), m_base(0), m_base_length(0)
{
        // To create by id, we fetch the block with an ACT that
        // populates m_easy_checksums and m_crypto_checksums.  It is
        // surely desirable to make it optional to fetch the the
        // DataBlock's themselves (so maybe the map should point to
        // BlockID's).
}


/*
  Set content, which might mean for the first time, in which case
  m_data_blocks, m_rolling_checksums, and m_crypto_checksums will all
  be empty.
*/
void CoverBlock::set_content(const string &in_contents)
{
        /* // sketch of function:
        assert(m_status & BlockStatus::Ready);
        long i_prev = -1;
        for(long i = 0; i < m_base_length - m_window_size, i++) {
                auto rolling_cs = compute_rolling_checksum(rolling_cs, i, i_prev);
                if(m_rolling_checksums.find(rolling_cs) != m_rolling_checksums.end()) {
                        // hit on rolling checksum
                        auto hard_cs = compute_crypto_checksum(i);
                        auto it = m_crypto_checksums.find(hard_cs);
                        if(it != m_crypto_checksums.end())
                                m_data_blocks.push_back(*it);
                }
        }
        // But we need to note the new offsets, so this isn't quite right
        */
}

/*
CoverBlock::compute_rolling_checksum(...);

CoverBlock::compute_crypto_checksum(...);
*/

/*
CoverBlock::rsync_algo_sketch()
{
        DataBlock *dbp = 0;
        long file_offset = 0;
        DataBlock *B[]; *B_new[];
        if(file_offset < B[dbp].start) {
                DataBlock *bp = make_block(offset);
                B_new.append(bp);
                file_offset += m_window_size;
        } else if(B[dbp + 1].start > file_offset) {
                B_new.append(B[dbp]);
        }
        return B_new;
}
*/



/******************************************************************************/
/* HeadBlock */

// No implementation currently needed for HeadBlock, just useful to
// exist in the inheritance hierarchy.



/******************************************************************************/
/* FileHeadBlock */


/*
  Create new based on contents
*/
FileHeadBlock::FileHeadBlock(const CreateByContent in,
                             const string &in_crypto_key,
                             const string &in_filename)
        : /*Head*/Block(CreateEmpty(), in_crypto_key)
//: m_remote_file(0)
{
        // Queue retrieval of remote file (if it exists).  ACT will
        // set the remaining bits in motion.

        // If remote file doesn't exist, instantiate trivial covering.
        // If remote file does exist and local has changed, compute minimal covering.
        // Queue any new covering blocks for staging and transfer.
        // Note that done on transfer ack.
}



/*
  Create new based on ID
*/
FileHeadBlock::FileHeadBlock(const CreateById in,
                             const string &in_crypto_key,
                             const BlockId &in_id)
        : /*Head*/Block(in, in_crypto_key, in_id)
{
        // Queue retrieval of remote file (if it exists).
        // If doesn't exist, flag error.
        // Else note that ready once ack.
}


FileHeadBlock::~FileHeadBlock()
{
}


TimelineBlock::HeadBlockPointer::HeadBlockPointer()
  : m_id(pseudo_random_string()),
    m_crypto_key(pseudo_random_string())
{
}


TimelineBlock::HeadBlockPointer::HeadBlockPointer(BlockId in_id, string &in_password)
        : m_id(in_id),
          m_crypto_key(in_password)
{
}
