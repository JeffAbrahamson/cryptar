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



#ifndef __CRYPTAR_H__
#define __CRYPTAR_H__ 1


#include <boost/thread.hpp>
#include <list>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>


namespace cryptar {

        /* ************************************************************ */
        /* Compression */

        std::string compress(const std::string &);
        std::string decompress(const std::string &, unsigned int = 0);


        /* ************************************************************ */
        /* Encryption */
        
        // Compute a hash (message digest).  Currently SHA-256.
        std::string message_digest(const std::string &message,
                                   const bool filesystem_safe = false);

        // Return a (not necessarily human readable) string of random bits.
        std::string pseudo_random_string(int length = 40);
        
        // Crypto++ is documented at http://www.cryptopp.com/
        // and http://www.cryptopp.com/fom-serve/cache/1.html
        // and http://www.cryptopp.com/wiki/FAQ

        std::string encrypt(const std::string &plain_message,
                            const std::string &password);
        std::string decrypt(const std::string &cipher_message,
                            const std::string &password);


        /* ************************************************************ */
        /* mode */

        /*
          Maintain a map of modes.
          
          Written for verbose and testing, to avoid having to pass them around
          everywhere in case we need them deep down.  Especially verbose.
  
          Some might call this a kludge or even inelegant.  Abused, it would be.
        */


        enum Mode {
                Verbose,
                Testing,
                Threads,        /* if false, do not start threads other than main */
        };

        void mode(const Mode m, const bool new_state);
        const bool mode(const Mode m);


        /* ************************************************************ */
        /* work units */

        /*
          A work unit is something that can be passed between a work
          thread and a communication thread.  The contract here is
          that the communication thread has control of the structure
          from the time the client calls hand_to_comm() until
          comm_done() returns true.

          This is probably obsolete, I think I rather want to derive
          from Block and pass on queues.
        */

        class WorkUnit {

        public:
                WorkUnit() {};
                virtual ~WorkUnit() {};

                void hand_to_comm() { m_comm_done = false; }  // should do handoff, too
                void comm_done() { m_comm_done = true; }    // should note in done queue
                bool is_comm() { return !m_comm_done; }

        private:
                // true if client has control, false if belongs to
                // communication thread
                bool m_comm_done;

                // Also needs reference or pointer to thread
                // and to (mutex-protected?) queue of things that are
                // done.
        };


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
        //class ACT_Base;


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
                Block(const std::string &in_crypto); // create empty (for use by derived classes)
                //Block(const std::string &in_crypto, const std::string &in_contents); // create new based on contents
                Block(const std::string &in_crypto_key, const BlockId in_id); // fetch based on block id
                virtual ~Block();

        public:
                bool action_pending() const { return !m_act_queue.empty(); }
                void completion_action(ACT_Base *);
                void completion_action();
                void write();

        protected:
                std::string m_cipher_text;
                const std::string m_crypto_key;
                BlockId m_id;        private:

                std::queue<ACT_Base *> m_act_queue;
        };


        class DataBlock : public Block {
        public:
                //DataBlock(const std::string &in_crypto_key, const BlockId id);
                DataBlock(const std::string &in_crypto_key, const std::string &in_data);       // create with content
                virtual ~DataBlock() {};

                std::string plain_text() const;

        private:
        };
        

        class HeadBlock : public Block {
        public:
                HeadBlock(const std::string &filename);
                HeadBlock(const BlockId id);
                virtual ~HeadBlock();
        };
        
        class FileHeadBlock : public HeadBlock {
        public:
                FileHeadBlock(const std::string &filename);
                FileHeadBlock(const BlockId id);
                virtual ~FileHeadBlock();

        };

        class DirectoryHeadBlock : public HeadBlock {
        public:
                DirectoryHeadBlock(const std::string &filename);
                DirectoryHeadBlock(const BlockId id);
                virtual ~DirectoryHeadBlock();
                
        };


        class TimeLineBlock : public Block {};
        
        class FileTimelineBlock : public TimeLineBlock {
        public:
                FileTimelineBlock(const BlockId id);
                virtual ~FileTimelineBlock();

                void trim_by_date(time_t when);
                void trim_by_count(int count);
        };


        class DirectoryTimelineBlock : public TimeLineBlock {
        public:
                DirectoryTimelineBlock(const std::string &filename);
                DirectoryTimelineBlock(const BlockId id);
                virtual ~DirectoryTimelineBlock();
                
                void trim_by_date(time_t when);
                void trim_by_count(int count);
        };


        /* ************************************************************ */
        /* Configuration */

        /*
          What to back up.
          Where to put it.
          How to get it there.
        */
        class Config {
        public:
                /*
                  How will this work?  Probably in real life, we'll
                  either create a new instance and populate from user
                  input or else create it from a name, which means
                  reading an (encrypted) file and populating the
                  fields that way.

                  How to represent and then compute the transfer and
                  staging info?
                */
                Config();    /* for making new configs */
                Config(const std::string &config_name);

                const std::string &local_dir() const { return m_local_dir; };
                void local_dir(const std::string &in) { m_local_dir = in; };

                const std::string &remote_dir() const { return m_remote_dir; };
                void remote_dir(const std::string &in) { m_remote_dir = in; };

                const std::string &remote_host() const { return m_remote_host; };
                void remote_host(const std::string &in) { m_remote_host = in; };

                std::string staging_dir() const;
                std::string push_to_remote() const;
                std::string pull_from_remote() const;

        private:
                std::string m_local_dir;
                std::string m_remote_dir;
                std::string m_remote_host;
        };

        
        /* ************************************************************ */
        /* For moving blocks to and from remote store */

        /*
          Staging involves persisting (or reading) a block to (or from) a local file.
          Transporting involves moving data to or from a remote host.

          The initial rsync-based mechanism writes files to a local
          staging directory before rsyncing them to the remote host.
          Other remote arrangements, such as various cloud providers,
          might involve different staging or transport mechanisms.
          Either (but not both of) staging or transport may be a
          no-op.
        */
        class Stage {
        public:
                Stage() {};
                virtual ~Stage() {};

                /*
                  Write a block such that the transport routines can
                  copy it to the remote store.  If transport doesn't
                  require staging to disk first, write() could also do
                  the transfer, in which case the corresponding
                  transport function would be a noop.
                */
                virtual void write(Block *bp) {
                        throw(std::logic_error("Calling Stage::write() directly"));
                              }
                /*
                  The reverse of write().  Instantiate a block that
                  has been fetched from the remote.  If the transport
                  routines don't need to stage to disk first, read()
                  could also do the transfer, in which case the
                  corresponding transport function would be a noop.
                 */
                virtual Block *read() {
                                throw(std::logic_error("Calling Stage::read() directly"));
                                      }

                /* Clean the staging area. */
                virtual void clean() {};
        };


        class StageFS : public Stage {
        public:
                StageFS(const std::string &in_base_dir);
                virtual ~StageFS() {};

                virtual void write(Block *bp);
                virtual Block *read();
                
        private:
                std::string m_base_dir;
        };


        /*
          The base transport class does nothing (i.e., no transport).
          This corresponds to using cryptar on a local store, as a
          sort of encrypted svn repository.  More usefully, it helps
          for testing.  Otherwise, a derived class is of more
          interest.
        */
        class Transport {
        public:
                Transport(Config &config) {};
                virtual ~Transport() {};

                // An action to take before pushing anything.
                virtual void pre_push() {};
                // An action to push a block to the remote store.
                virtual void push(Block *bp) {};
                // An action to take after all blocks are pushed.
                virtual void post_push() {};
                
                // An action to take before pulling anything.
                virtual void pre_pull() {};
                // An action to pull a block to the remote store.
                virtual void pull(Block *bp) {};
                // An action to take after all blocks are pulled.
                virtual void post_pull() {};

        protected:
                Config m_config;
        };


        class TransportRsync {
        public:
                TransportRsync(Config &config) {};
                virtual ~TransportRsync() {};

                // An action to take before pushing anything.
                virtual void pre_push() {};
                // An action to push a block to the remote store.
                virtual void push(Block *bp) {};
                // An action to take after all blocks are pushed.
                virtual void post_push() {};
                
                // An action to take before pulling anything.
                virtual void pre_pull() {};
                // An action to pull a block to the remote store.
                virtual void pull(Block *bp) {};
                // An action to take after all blocks are pulled.
                virtual void post_pull() {};
        };
        

        /* ************************************************************ */
        /* Communicator */

        const int communicator_prod_batch_size = 100;
        const int communicator_test_batch_size = 3;

        class Communicator {
        public:
                Communicator(const Stage *in_stage, const Config *in_config);
                ~Communicator();

                void push(Block *);
                void wait();
                void operator()();   // Should really only be called at thread creation

        private:
                typedef std::queue<Block *>::size_type queue_size_type;
                
                void run();
                bool queue_empty();
                Block *pop();
                void comm_batch();

                const queue_size_type m_batch_size;
                /*
                  If these next are values instead of pointer (or
                  reference, but that makes instantiating this
                  annoying), then we slice the objects to the base.
                  So just use pointers.
                */
                const Stage *m_stage;
                const Config *m_config;

                bool m_needed;    /* set to false to encourage auto-shutdown */
                boost::mutex m_queue_access;
                std::queue<Block *> m_queue;
                boost::thread *m_thread;
        };

        
        
        /* ************************************************************ */
        /* The filesystem in the remote store. */


        /*
          Stat structure for remote store files.  This will
          (eventually) have to deal with different local file systems
          and OS's, say to support MacOS and Windows.
         */
        class StatInfo {};

        
        /*
          Filesystem node base class.
        */

        class FS_Node {
        public:
                FS_Node();
                virtual ~FS_Node();

                FS_Node &get_parent() const;
                TimeLineBlock &get_timeline() const;
                HeadBlock &get_head() const;
                StatInfo &stat() const;
                bool is_dirt() const;
                void persist();
        };


        class FS_File : public FS_Node {};

        class FS_Dir : public FS_Node {
        public:
                FS_Dir();
                virtual ~FS_Dir();

                /* Perhaps these two should add FS_Node's, and so
                   perhaps even be the same function?
                */
                void add_dir(std::string &dir_name);
                void add_file(std::string &file_name);

                std::vector<FS_Node &> get_entries() const;
        };

        /*
          A representation of the filesystem in the remote store.
        */
        
        class FileSystem {
        public:
                FileSystem(BlockId id);
                ~FileSystem();

                FS_Node &find_by_name(std::string &filename);
                FS_Node &find_by_hash(std::string &hash);

                void persist();
                
        private:
                // It's worth testing if these should be std::hash instead of std::map
                // Map filename or file hash to vector of paths where found.
                // A hash could map to a vector of length greater than one if
                // a file has been copied.
                std::map<std::string, std::vector<std::string> > m_names;
                std::map<std::string, std::vector<std::string> > m_hashes;

                /* Base directory for remote file system.
                */
                FS_Dir m_dir;
                
                /* Time for which the remote instance is instantiated.
                   If zero, then time is max time in each TimeLine.
                */
                time_t m_when;

        };


        /* ************************************************************ */
        /* ACT's -- Asynchronous Completion Tokens. */

        
        /*
          The base class, a noop ACT.
        */ /*
        class ACT_Base {
                ACT_Base() {};
                virtual ~ACT_Base() {};

                //virtual void operator()() = 0;
                virtual void operator()() { throw std::logic_error("operator() in ACT_Base"); }
        };
        */

        /*
          We might say "object" rather than "token" but for usage by
          Douglas C. Schmidt, 1998, 1999:
          http://www.cs.wustl.edu/~schmidt/PDF/ACT.pdf
         */

        /*
          ACT that selects a Head object from a TimeLine and queues
          fetching it.
        */
        class TimeLine_HeadSelector : public ACT_Base {
                // Select most recent Head less than or equal to time.
                // If time is zero, select the most recent Head.
                TimeLine_HeadSelector(TimeLineBlock *tlb, time_t time);
                ~TimeLine_HeadSelector();

                virtual void operator()();
        };


        /*
          ACT that fetches the blocks of a head object.
          If levels > 0, recursively fetch sub-objects.
          If the Head object does not represent a directory,
          levels has no effect.
        */
        class Head_FetchBlocks : public ACT_Base  {
                Head_FetchBlocks(HeadBlock *hb, int levels = 0);
                ~Head_FetchBlocks();

                virtual void operator()();
        };


        /*
          ACT that inserts a file into a filesystem object.
        */
        class Head_InsertFS : public ACT_Base  {
                Head_InsertFS(HeadBlock *hb, FS_Node *fsn);
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
          ACT to call a trigger on a Head or TimeLine object.
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

#endif  /* __CRYPTAR_H__*/
