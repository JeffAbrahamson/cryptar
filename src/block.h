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


#include <map>
#include <stdexcept>
#include <string>
#include <queue>



namespace cryptar {

        /* ************************************************************ */
        /* ACT_Base -- Asynchronous Completion Tokens. */

        
        /*
          The base class, a noop ACT.
          The action is invoked by operator().
        */
        class ACT_Base {
        public:
                ACT_Base() {};
                virtual ~ACT_Base() {};

                //virtual void operator()() = 0;
                virtual void operator()() { throw std::logic_error("operator() in ACT_Base"); }
        };


        
        /* ************************************************************ */
        /* blocks of various sorts */

        /*
          Blocks come in several flavors.
          In one dimenions, they represent files or directories.
          
          In another dimension, they represent pieces of files, images
          (collections of pieces), or timelines (sequences of images).
        */

        typedef std::string BlockId;

        
        /*
          Basic block, from which all blocks derive.
          I don't think anything should need to instantiate this directly,
          for which reason the constructors are protected.
        */
        class Block {
                // I don't think anything should need to instantiate this directly,
                // for which reason constructor and destructor should
                // surely be protected.  But for the moment,
                // communicate_test.cpp does, and I'm not yet sure what the best answer
                // is.
        public:
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
                void write(const std::string &in_dir, bool flat = false) const;
                void read(const std::string &in_dir, bool flat = false);

                const BlockId &id() const { return m_id; }
                
        protected:
                std::string m_cipher_text;
                const std::string m_crypto_key;
                BlockId m_id;
                bool m_dirty;   /* If true, block has been modified since fetch from store */
                bool m_ready;   /* If false, block is being fetched, no data is valid. */
                
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


        /*
          Block ID's and contents are both strings, so use factory
          functions to hide some notation.
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

        private:
        };
        

        class InitBlock : public Block {
        public:
                // CreateByContent may not make sense except empty?
                InitBlock(const CreateByContent,
                          const std::string &in_crypto_key,
                          const std::string &in_data);
                InitBlock(const CreateById,
                          const std::string &in_crypto_key,
                          const BlockId &in_id);
                virtual ~InitBlock() {};

                BlockId root_id(const std::string &in_name, const bool in_create = false);
        };


        /*
        class HeadBlock : public Block {
        public:
                HeadBlock(const CreateByContent,
                          const std::string &in_crypto_key,
                          const std::string &in_filename);
                HeadBlock(const CreateById,
                          const std::string &in_crypto_key,
                          const BlockId &in_id);
                virtual ~HeadBlock();
        };
        */


        typedef unsigned long WeakChecksum;
        typedef std::string StrongChecksum;

        
        class FileHeadBlock : public /*Head*/Block {
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


        class DirectoryHeadBlock : public /*Head*/Block {
        public:
                DirectoryHeadBlock(const CreateByContent,
                                   const std::string &in_crypto_key,
                                   const std::string &filename);
                DirectoryHeadBlock(const CreateById,
                                   const std::string &in_crypto_key,
                                   const BlockId &in_id);
                virtual ~DirectoryHeadBlock();
                
        };


        enum FileType {
                // Do not renumber members of this enum.  Values are persisted.
                Invalid = 0,
                Regular = 1,
                Directory = 2,
                SymLink = 3,    /* Not yet supported */
                Socket = 4,     /* Not yet supported */
                Door = 5,       /* Solaris, not supported */
                BlockSpecial = 6, /* Not supported */
                CharacterSpecial = 7, /* Not supported */
                Pipe = 8,             /* Not yet supported */
        };

        // Is there really a difference between File and Directory TimelineBlock's ?
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
        

        // Obsolete, I think
        /*
        class FileTimelineBlock : public TimelineBlock {
        public:
                FileTimelineBlock(const CreateById, const BlockId &in_id);
                virtual ~FileTimelineBlock();

                void trim_by_date(time_t when);
                void trim_by_count(int count);
        };


        class DirectoryTimelineBlock : public TimelineBlock {
        public:
                DirectoryTimelineBlock(const CreateByContent, const std::string &filename);
                DirectoryTimelineBlock(const CreateById, const BlockId &in_id);
                virtual ~DirectoryTimelineBlock();
                
                void trim_by_date(time_t when);
                void trim_by_count(int count);
        };
        */
}

#endif  /* __BLOCK_H__*/
