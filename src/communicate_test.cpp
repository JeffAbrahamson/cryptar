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



#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE tests
#include <boost/test/unit_test.hpp>

#include "cryptar.h"


using namespace cryptar;
using namespace std;


namespace {

        class ACT_Print : public ACT_Base {

        public:
                ACT_Print(int i) : m_i(i) {};
                virtual void operator()()
                {
                        cout << "Completing " << m_i << endl;
                        BOOST_CHECK(m_i > 0);
                }

        private:
                int m_i;
        };
        

        /*
          Queue some blocks for transfer.
          Their completion methods just print that they've been transferred.
        */
        void print_completion(bool thread)
        {
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, thread);

                ConfigParam params(no_transport);
                params.m_passphrase = pseudo_random_string();
                params.m_local_dir = "/tmp/cryptar-block-test-"
                        + filename_from_random_bits();
                shared_ptr<Config> config = make_config(params);
                
                /*
                  Start with 1 so that we can verify that ACT's have
                  been initialized.  Since we've specified
                  no_transport, we don't actually do anything with the
                  data.  We're just testing that the ACT's are
                  properly poked so that we know when all is done.
                */
                string pass = ""; // value doesn't matter, we're not moving data
                for(int i = 1; i <= 10; i++) {
                        /* We can't allocate a base block (it is
                           abstract), so use DataBlock for our test,
                           as it is simple and adds minimally to
                           Block.
                        */
                        DataBlock *bp = block_by_content<DataBlock>(params.transport(),
                                                                    pass,
                                                                    string());
                        bp->completion_action(new ACT_Print(i));
                        //config->sender()->push(bp);
                        bp->write();
                }

                // process the communication queue, once if thread == false,
                // completely otherwise.
                if(thread)
                        ;//config->sender()->wait();
                else
                        ;//config->sender();
        }
}


BOOST_AUTO_TEST_CASE(case_print_completion_one)
{
        print_completion(false);
}


BOOST_AUTO_TEST_CASE(case_print_completion_thread)
{
        print_completion(true);
}

