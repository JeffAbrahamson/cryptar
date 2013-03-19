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

#if 0
#include <map>
#include <string>
#include <vector>

#include "filesystem.h"


using namespace cryptar;
using namespace std;


/*
  We talk about *the* remote filesystem.  Indeed, the design assumes a
  unique remote file system.  If in the future we choose to support
  more than one, we'll want to make the remote filesystem
  representation live in thread-local storage.  It is very unclear at
  the moment that this extra complexity will be worthwhile.
*/


FS_Node::FS_Node()
{
}



FS_Node::~FS_Node()
{
}

FS_Node &FS_Node::get_parent() const;
TimeLineBlock &FS_Node::get_timeline() const;
HeadBlock &FS_Node::get_head() const;
StatInfo &FS_Node::stat() const;
bool FS_Node::is_dirty() const;
void FS_Node::persist();




FS_Dir::FS_Dir();
FS_Dir::~FS_Dir();
void FS_Dir::add_dir(std::string &dir_name);
FS_Dir::add_file(std::string &file_name);
vector<FS_Node &> FS_Dir::get_entries() const;


/*
  Initialize the local image of the remote file system.
  We cache all directory nodes, but not leaf (file) contents.

  Perhaps this should be called RemoteFileSystem?  FIXME
*/
FileSystem::FileSystem(const Config &in_config)
        : m_config(in_config)
{
        m_num_pending = 1;
        InitBlock *init_bp = block_by_id<InitBlock>(pass, config.init_id);
        while(m_num_pending > 0) {
                while(queue.empty())
                        ;       // could nanosleep?  Or mutex on queue?  Or atomic?
                // FIXME where does queue come from?
                // Do we derive from Communicator?
                // How does comm thread know how to find our queue?  An ACT?
                Block *bp = queue.front();
                queue.pop();
                // Subtract one (processed this block) and add whatever new we've pushed
                m_num_pending += process_remote_tree_node(bp) - 1;
        }
}



FileSystem::FileSystem(const BlockId &id);
FileSystem::~FileSystem();

FS_Node &FileSystem::find_by_name(string &filename);
FS_Node &FileSystem::find_by_hash(string &hash);

void FileSystem::persist();


/*
  process_remote_tree_node(...)
  We've received a block from the communication thread.
  Do with it what we must.  Return how many more nodes we've
  requested, if any.
*/
unsigned int FileSystem::process_remote_tree_node(EmptyBlock *bp)
{
        // This isn't necessarily an error, but we should make clear
        // later under what circumstances this is really possible
        // without being an error.  In any case, it represents a
        // request for a block that does not exist on the remote
        // store, so we probably want to log it somehow.
        if(mode(Verbose))
                cout << "Received EmptyBlock in FileSystem::process_remote_tree_node." << endl;
        return 0;
}


unsigned int FileSystem::process_remote_tree_node(InitBlock *bp)
{
        BlockID id = bp->root_id(m_conf.root_name);
        TimeLineBlock *bp = block_by_id<TimeLineBlock>(m_conf.crypto_key, id);
        return 1;
}


unsigned int FileSystem::process_remote_tree_node(TimeLineBlock *bp)
{
        BlockId id = bp->select_head(m_conf.m_threshold_time);
        switch(bp->file_type())
        case FileType::Regular:
                block_by_id<FileHeadBlock>(
}


unsigned int FileSystem::process_remote_tree_node(FileHeadBlock *bp)
{
}


unsigned int FileSystem::process_remote_tree_node(DirectoryHeadBlock *bp)
{
}

#endif
