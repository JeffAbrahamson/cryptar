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
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "stage.h"


using namespace cryptar;
using namespace std;



/*
  Factory method to build stage objects from stage types.
*/
Stage *cryptar::make_stage(StageType in_stage_type, const string &in_base_dir)
{
        switch(in_stage_type) {
        case stage_invalid:
                {
                        // Why do I see an error if this block is not separately scoped?
                        ostringstream error_message("Unexpected staging type, ");
                        error_message << in_stage_type;
                        throw(runtime_error(error_message.str()));
                }
        case base_stage:
                /*
                {
                        // Why do I see an error if this block is not separately scoped?
                        ostringstream error_message("Unexpected (but known) staging type, ");
                        error_message << in_stage_type;
                        throw(runtime_error(error_message.str()));
                }
                */
                // Remove preceding comment or make Stage() constructor protected.  Probably the former.
                return new Stage();
                break;                
        case stage_out_fs:
                return new StageOutFS(in_base_dir);
        case stage_in_fs:
                return new StageInFS(in_base_dir);
        }
        ostringstream error_message;
        error_message << "Unknown staging type, " << in_stage_type;
        throw(runtime_error(error_message.str()));
}


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
                // FIXME:  should try harder (mkdir -p)
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
                // FIXME:  should try harder (mkdir -p)
        }
}



void StageInFS::operator()(Block *bp) const
{
        assert(bp);
        bp->read(m_base_dir);
}



