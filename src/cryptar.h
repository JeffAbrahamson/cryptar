/*
  Copyright 2012  Jeff Abrahamson
  
  This file is part of cryptar.
  
  This software might be free, it might be commercial.
  You may safely assume that using it for personal, non-commercial
  use will remain acceptable.  The eventual license may, however, be a
  split free/commercial license, such as the one long used by
  Sleepycat.

  In any case, this software is explicitly licensed the terms of the
  GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any
  later version.
  
  cryptar is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with cryptar.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef __CRYPTAR_H__
#define __CRYPTAR_H__ 1


#include <boost/thread.hpp>
#include <list>
#include <map>
#include <queue>
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
        std::string pseudo_random_string(int length);
        
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
        /* blocks of various sorts */

        /*
          Blocks come in several flavors.
          In one dimenions, they represent files or directories.
          
          In another dimension, they represent pieces of files, images
          (collections of pieces), or timelines (sequences of images).
        */
        
        typedef unsigned int BlockId;


        class Block {

        public:
                Block();                            // create empty
                Block(const std::string &contents); // create new based on contents
                Block(const BlockId id);            // fetch based on block id
                virtual ~Block();

                void write();
                
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
        /* For moving blocks to and from remote store */

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
        };


        /* ************************************************************ */
        /* Communicator */

        class Communicator {
        public:
                Communicator();
                ~Communicator();

                void push(Block *);
                void operator()();   // Should really only be called at thread creation

        private:
                typedef std::queue<Block *>::size_type queue_size_type;
                
                void run();
                bool queue_empty();
                Block *pop();
                void comm_batch();

                bool m_needed;    /* set to false to encourage auto-shutdown */
                const queue_size_type m_batch_size;
                boost::mutex m_queue_access;
                std::queue<Block *> m_queue;
                boost::thread *m_thread;
                Stage *m_stage;
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
          Noop ACT.
        */
        class DoNothing {};


        /*
          We might say "object" rather than "token" but for usage by
          Douglas C. Schmidt, 1998, 1999:
          http://www.cs.wustl.edu/~schmidt/PDF/ACT.pdf
         */

        /*
          ACT that selects a Head object from a TimeLine and queues
          fetching it.
        */
        class TimeLine_HeadSelector {
                // Select most recent Head less than or equal to time.
                // If time is zero, select the most recent Head.
                TimeLine_HeadSelector(TimeLineBlock *tlb, time_t time);
                ~TimeLine_HeadSelector();
        };


        /*
          ACT that fetches the blocks of a head object.
          If levels > 0, recursively fetch sub-objects.
          If the Head object does not represent a directory,
          levels has no effect.
        */
        class Head_FetchBlocks {
                Head_FetchBlocks(HeadBlock *hb, int levels = 0);
                ~Head_FetchBlocks();
        };


        /*
          ACT that inserts a file into a filesystem object.
        */
        class Head_InsertFS {
                Head_InsertFS(HeadBlock *hb, FS_Node *fsn);
                ~Head_InsertFS();
        };


        /*
          ACT to note that a block has been fetched.
        */
        class Block_NoteFetched {
                Block_NoteFetched(Block *b);
                ~Block_NoteFetched();
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
        class Block_Trigger {
                Block_Trigger(Block *b, Block *other);
                ~Block_Trigger();
        };
}

#endif  /* __CRYPTAR_H__*/
