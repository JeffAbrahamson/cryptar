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


#define __FILESYSTEM_H__ 1      /* FIXME    temp hack */
#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__ 1


#include <map>
#include <string>
#include <vector>

#include "block.h"


namespace cryptar {

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
                TimelineBlock &get_timeline() const;
                /*Head*/Block &get_head() const;
                StatInfo &stat() const;
                bool is_dirty() const;
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

        class Config;           /* forward declaration, cf. config.h */

        class FileSystem {
        public:
                FileSystem(const Config &config);
                FileSystem(const BlockId &id);
                ~FileSystem();

                FS_Node &find_by_name(std::string &filename);
                FS_Node &find_by_hash(std::string &hash);

                void persist();

        private:
                //unsigned int process_remote_tree_node(EmptyBlock *bp);
                //unsigned int process_remote_tree_node(InitBlock *bp);
                unsigned int process_remote_tree_node(TimelineBlock *bp);
                unsigned int process_remote_tree_node(FileHeadBlock *bp);
                unsigned int process_remote_tree_node(DirectoryHeadBlock *bp);


                // It's worth testing if these should be std::hash instead of std::map
                // Map filename or file hash to vector of paths where found.
                // A hash could map to a vector of length greater than one if
                // a file has been copied.
                std::map<std::string, std::vector<std::string> > m_names;
                std::map<std::string, std::vector<std::string> > m_hashes;

                Config &m_config;

                /* Base directory for remote file system.
                */
                FS_Dir m_dir;

                /* Time for which the remote instance is instantiated.
                   If zero, then time is max time in each TimeLine.
                */
                time_t m_when;

        };
}

#endif  /* __FILESYSTEM_H__*/
