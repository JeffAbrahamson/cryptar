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



#ifndef __BLOCK_H__
#define __BLOCK_H__ 1


#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <queue>
#include <vector>

#include "crypt.h"


namespace cryptar {

        /* ************************************************************ */
        /* BlockId */

        /*
          Blocks come in several flavors.
          In one dimension, they represent files or directories.
          
          In another dimension, they represent pieces of files, images
          (collections of pieces), or timelines (sequences of images).
        */

        class BlockId {
        public:
                BlockId() { m_id = pseudo_random_string(); }
                BlockId(const std::string in_id)
                        : m_id(in_id) {};
                ~BlockId() {};

                bool operator==(const BlockId &in_id) const
                {
                        return this->m_id == in_id.id();
                }
                bool operator!=(const BlockId &in_id) const
                {
                        return !operator==(in_id);
                }

                // Don't permit accidental conversion to string.
                const std::string &as_string() const { return m_id; };
                const bool empty() const { return m_id.empty(); };

        private:
                const std::string &id() const { return m_id; };

                std::string m_id;
                
                friend class boost::serialization::access;
                template<class Archive>
                        void serialize(Archive &in_ar, const unsigned int in_version) {
                        in_ar & m_id;
                }
        };

        
        /* ************************************************************ */
        /* ACT_Base -- Asynchronous Completion Tokens. */

        class Block;
        
        /*
          The base class, a noop ACT.
          The action is invoked by operator().

          That is, on completion of the task, the task completer
          should have an ACT_Base reference A and call A() to signal
          completion.
        */
        class ACT_Base {
        public:
                ACT_Base() {};
                virtual ~ACT_Base() {};

                // Three kinds of completions.
                // In the first case, we've done what was reqeusted, there's no return.
                // In the second case, we requested a BlockId.
                // In the third, we requested a block.
                virtual void operator()()
                { throw std::logic_error("operator()() in ACT_Base"); }
                
                virtual void operator()(const BlockId &)
                { throw std::logic_error("operator()(const BlockId &) in ACT_Base"); }
                
                virtual void operator()(const Block *)
                { throw std::logic_error("operator()(const Block *) in ACT_Base"); }
        };

        
        /* ************************************************************ */
        /* blocks of various sorts */

        /*
          Basic block, from which all blocks derive.
          Nothing instantiates this.
        */
        class Block {
        public:
                enum BlockStatus {
                        block_status_invalid = 0x0,   // Block is being fetched, no data is valid
                        ready = 0x1,                  // Data is fetched, block may be used
                        dirty = 0x2,                  // Data has been modified but not yet synced
                                                      // back to the store.
                        not_found = 0x4,              // Block was not found in store
                };

                struct CreateEmpty {};
                struct CreateById {};
                struct CreateByContent{};
                
                // create empty (for use by derived classes)
                Block(const CreateEmpty,
                      const std::string &in_crypto);
                Block(const CreateEmpty,
                      const std::string &in_crypto,
                      std::string &in_persist_dir);
                /*
                // create new based on contents
                Block(const std::string &in_crypto, const std::string &in_contents);
                */
                // fetch based on block id
                Block(const CreateById,
                      const std::string &in_crypto_key,
                      const BlockId &in_id);
                Block(const CreateById,
                      const std::string &in_crypto_key,
                      const BlockId &in_id,
                      std::string &in_persist_dir);
                virtual ~Block();

        public:                 /* most should be protected? */
                bool action_pending() const { return !m_act_queue.empty(); }
                void completion_action(ACT_Base *);
                void completion_action();
                /* FIXME:    (Deprecate write() and read().) */
                void write(const std::string &in_dir, bool flat = false) const;
                void read(const std::string &in_dir, bool flat = false);
                /* to_string() serializes the block and returns the string */
                virtual const std::string to_stream() const = 0;
                /* from_string() sets the state of the block given a serialized version */
                virtual void from_stream(const std::string &in_string) = 0;

                const BlockId &id() const { return m_id; }
                
        protected:
                std::string m_cipher_text;      /* encrypted contents of this block */
                const std::string m_crypto_key; /* cryptographic key for this block */
                BlockId m_id;                   /* identifier (in filesystem) for this block */
                BlockStatus m_status;           /* status of this block */

        private:
                /*
                  When we are asked to persist, we are handed a
                  directory to which we should persist.  A staging
                  object may desire that blocks distribute themselves
                  in a hierarchy, in which case write() is called with
                  flat == false.  Then we use m_persist_dir in
                  addition to the staging directory requested by the
                  caller of write().
                */
                const std::string m_persist_dir;

                std::queue<ACT_Base *> m_act_queue;

                std::string id_to_pathname(const std::string &in_dir, bool flat) const;
        };
        typedef Block::BlockStatus BlockStatus;
        
        inline BlockStatus operator|(BlockStatus a, BlockStatus b) {
                // The + avoids recursive call to operator|().
                // http://stackoverflow.com/questions/4226960/type-safer-bitflags-in-c
                return static_cast<BlockStatus>(+a | +b);
        }
        

        /*
          Block ID's and contents are both strings, so use factory
          functions to hide some notation.
          FIXME  (This is no longer true.  But probably still want factory functions.)
          FIXME  (Should blocks be shared_ptr's?)
        */
        template<typename T> T *block_empty(const std::string &in_crypto)
                {
                        return new T(Block::CreateEmpty(), in_crypto);
                }

        template<typename T> T *block_by_content(const std::string &in_crypto,
                                                 const std::string &in_content)
                {
                        return new T(Block::CreateByContent(), in_crypto, in_content);
                }
        template<typename T> T *block_by_content(const std::string &in_crypto,
                                                 const std::string &in_content,
                                                 const std::string &in_persist_dir)
                {
                        return new T(Block::CreateByContent(), in_crypto, in_content, in_persist_dir);
                }
        template<typename T> T *block_by_id(const std::string &in_crypto,
                                            const BlockId &in_id)
                {
                        return new T(Block::CreateById(), in_crypto, in_id);
                }
        template<typename T> T *block_by_id(const std::string &in_crypto,
                                            const BlockId &in_id,
                                            const std::string &in_persist_dir)
                {
                        return new T(Block::CreateById(), in_crypto, in_id, in_persist_dir);
                }


        // Cf. also config.h.  Config objects are blocks as well, but are sufficiently
        // specialized they don't really belong here.

        /*
          A block that simply persists and restores itself (with encryption).
          It has no idea of internal structure.  It is a leaf node in the tree.
        */
        class DataBlock : public Block {
        public:
                DataBlock(const CreateByContent,
                          const std::string &in_crypto_key,
                          const std::string &in_data);
                DataBlock(const CreateById,
                          const std::string &in_crypto_key,
                          const BlockId &id);
                virtual ~DataBlock() {};

                std::string plain_text() const;
                void set_content(const std::string &in_contents);

                /* to_stream() serializes the block and returns the string */
                virtual const std::string to_stream() const
                { return m_cipher_text; }
                /* from_stream() sets the state of the block given a serialized version */
                virtual void from_stream(const std::string &in_stream)
                { m_cipher_text =in_stream; }

        private:
        };
        


        /*
          A block whose data is too big to push as a single chunk, so
          it computes a covering of smaller blocks.  This block's data
          is the set of block id's (pointing to DataBlocks) and
          offsets that form the covering.  This block is what
          implements the cryptar algorithm.

          Not implemented yet: If the covering is itself too large, we
          could store the covering in another CoverBlock and here
          store only a pointer to the secondary CoverBlock.
        */
        class CoverBlock : public DataBlock {
        public:
                CoverBlock(const CreateByContent,
                          const std::string &in_crypto_key,
                          const std::string &in_data);
                CoverBlock(const CreateById,
                          const std::string &in_crypto_key,
                          const BlockId &in_id);
                //virtual ~CoverBlock();

                void set_content(const std::string &in_contents);

        private:
                // Should window size really be compiled into the program?
                static const unsigned int m_window_size; /* rsync window size, in bytes */
                
                // Point ourselves at local data.  To provide or
                // consume the local data, derive from CoverBlock.
                char *m_base;
                unsigned long m_base_length;

                // Remote data
                std::vector<DataBlock *> m_data_blocks;
                std::set<unsigned long> m_easy_checksums;
                std::map<unsigned long, DataBlock *> m_crypto_checksums;
        };
        

        typedef unsigned long WeakChecksum;
        typedef std::string StrongChecksum;

#ifdef LATER
        /*
          Describe a file.  Contains file meta-information and
          pointers to the contents.  Maybe this derives from
          CoverBlock instead?
        */
        class FileBlock : public Block {
        public:
                FileHeadBlock(const CreateByContent,
                              const std::string &in_crypto_key,
                              const std::string &filename);
                FileHeadBlock(const CreateById,
                              const std::string &in_crypto_key,
                              const BlockId &in_id);
                virtual ~FileHeadBlock();

        private:
                struct BlockTuple {
                        const BlockId m_id;
                        const WeakChecksum m_wcs;
                        const StrongChecksum m_scs;
                        const size_t base_offset;
                        const size_t length;
                        const std::string password;
                };
                std::vector<BlockTuple> m_remote_blocks; /* what we serialise to m_cipher_text */
                // Resolve what I mean here, as filesystem.h is currently included after block.h
                // FS_File *m_remote_file; /* NULL if not yet instantiated. */
        };


        /*
          Describes a directory.  Contains a map of filenames to
          directory-level info about files and the block id's where
          each file lies.
        */
        class DirectoryHeadBlock : public Block {
        public:
                DirectoryHeadBlock(const CreateEmpty,
                                   const std::string &in_crypto_key);
                DirectoryHeadBlock(const CreateByContent,
                                   const std::string &in_crypto_key,
                                   const std::string &in_filename);
                DirectoryHeadBlock(const CreateById,
                                   const std::string &in_crypto_key,
                                   const BlockId &in_id);
                virtual ~DirectoryHeadBlock() {};
                
        };


        enum FileType {
                // Do not renumber members of this enum.  Values are persisted.
                file_type_invalid = 0,
                regular = 1,
                directory = 2,
                symlink = 3,           /* Not yet supported */
                socket = 4,            /* Not yet supported */
                door = 5,              /* Solaris, not supported */
                block_special = 6,     /* Not supported */
                character_special = 7, /* Not supported */
                pipe = 8,              /* Not yet supported */
        };


        /*
          Describe multiple revisions of a block.
          This probably should be dropped, at least for now.
          Doing everything gets heavy.
        */
        class TimelineBlock : public Block {
        public:
                struct HeadBlockPointer {
                        HeadBlockPointer();
                        HeadBlockPointer(BlockId in_id, std::string &in_password);

                        const BlockId m_id;
                        const std::string m_crypto_key;
                };

                TimelineBlock(const CreateByContent,
                              const std::string &in_crypto_key,
                              const std::string &in_filename,
                              const std::string &in_stream);
                TimelineBlock(const CreateById,
                              const std::string &in_crypto_key,
                              const BlockId &in_id);
                virtual ~TimelineBlock();

                // Select the most recent head block with time less than or equal to in_when.
                // Zero means most recent.
                const BlockId *select_head(time_t in_when = 0);

                // Remove head blocks before in_when
                void trim_by_time(time_t in_when);

                // Remove all but in_count most recent head blocks
                void trim_by_count(int in_count);

                const FileType file_type() const { return m_file_type; }

        private:
                // Begin persisted data
                std::map<const time_t, const HeadBlockPointer> m_head_blocks;
                FileType m_file_type; /* requires support in FileSystem.cpp */
                // End persisted data
        };
#endif  /* LATER */
}

#endif  /* __BLOCK_H__*/
