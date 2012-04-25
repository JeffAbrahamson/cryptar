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


#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cryptar.h"


using namespace cryptar;
using namespace std;



/*
  Directory where we should stage files.
*/
StageOutFS::StageOutFS(const string &in_base_dir)
        : m_base_dir(in_base_dir)
{
        if(mkdir(m_base_dir.c_str(), 0700) && EEXIST != errno) {
                cerr << "  Error creating directory \""
                     << m_base_dir << "\": " << strerror(errno) << endl;
                throw(runtime_error("Failed to create staging directory."));
                // ################ should try harder (mkdir -p)
        }
}



void StageOutFS::operator()(Block *bp) const
{
        assert(bp);
        bp->write(m_base_dir);
}



/*
  Directory where we should stage files.
*/
StageInFS::StageInFS(const string &in_base_dir)
        : m_base_dir(in_base_dir)
{
        if(mkdir(m_base_dir.c_str(), 0700) && EEXIST != errno) {
                cerr << "  Error creating directory \""
                     << m_base_dir << "\": " << strerror(errno) << endl;
                throw(runtime_error("Failed to create staging directory."));
                // ################ should try harder (mkdir -p)
        }
}



void StageInFS::operator()(Block *bp) const
{
        assert(bp);
        bp->read(m_base_dir);
}



