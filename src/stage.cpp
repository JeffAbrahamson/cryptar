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



#include "cryptar.h"



/*
  Directory where we should stage files.
*/
StageFS::StageFS(const string &in_base_dir)
        : m_base_dir(in_base_dir)
{
        if(mkdir(m_dir_name.c_str(), 0700) && EEXIST != errno) {
                cerr << "  Error creating directory \""
                     << m_dir_name << "\": " << strerror(errno) << endl;
                throw(runtime_error("Failed to create staging directory."));
                // ################ should try harder (mkdir -p)
        }
}



void StageFS::write(Block *bp)
{
        
}



Block *StageFS::read()
{
}

