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



#ifndef __STAGE_H__
#define __STAGE_H__ 1


#include "block.h"
#include "config.h"


namespace cryptar {

        /*
          Staging involves persisting (or reading) a block to (or
          from) a local file.

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

                virtual StageType stage_type() { return base_stage; }
                /*
                  Write a block such that the transport routines can
                  copy it to the remote store.  If transport doesn't
                  require staging to disk first, write() could also do
                  the transfer, in which case the corresponding
                  transport function would be a noop.

                  Alternatively, read a block based on the BlockId.
                  The reverse of write().  Instantiate a block that
                  has been fetched from the remote.  If the transport
                  routines don't need to stage to disk first, read()
                  could also do the transfer, in which case the
                  corresponding transport function would be a noop.
                */
                virtual void operator()(Block *in_bp) const {
                        throw(std::logic_error("Calling Stage::operator()() directly"));
                              }

                /* Clean the staging area. */
                virtual void clean() {};
        };


        Stage *make_stage(enum StageType in_stage_type, const std::string &in_base_dir);


        class StageOutFS : public Stage {
        public:
                StageOutFS(const std::string &in_base_dir);
                virtual ~StageOutFS() {};

                virtual StageType stage_type() { return stage_out_fs; }
                virtual void operator()(Block *in_bp) const;
                
        private:
                std::string m_base_dir;
        };


        class StageInFS : public Stage {
        public:
                StageInFS(const std::string &in_base_dir);
                virtual ~StageInFS() {};

                virtual StageType stage_type() { return stage_in_fs; }
                virtual void operator()(Block *in_bp) const;
                
        private:
                std::string m_base_dir;
        };
}


#endif  /* __STAGE_H__*/
