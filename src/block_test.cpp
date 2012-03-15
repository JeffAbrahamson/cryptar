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
//#include <algorithm>
#include <boost/test/unit_test.hpp>
//#include <pstreams/pstream.h>
//#include <string>

#include "cryptar.h"
#include "test_text.h"


using namespace cryptar;
using namespace std;


namespace {

        int num_completions;


        class ACT_Check : public ACT_Base {

        public:
                ACT_Check(DataBlock *in_bp, string in_orig)
                        : m_bp(in_bp), m_original_plain_text(in_orig) {};
                virtual void operator()()
                {
                        BOOST_CHECK(m_original_plain_text == m_bp->plain_text());
                        ++num_completions;
                }

        private:
                DataBlock *m_bp;
                string m_original_plain_text;
        };
        

        /*
          Queue some blocks for transfer.
          Their completion methods just check that encryption is working.
        */
        void check_completion(bool thread)
        {
                num_completions = 0;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, thread);
                Communicator c(new NoStage(), new Config());

                // Start with 1 so that we can verify that ACT's have
                // been initialized.
                const int loop_num = 10;
                for(int i = 1; i <= loop_num; i++) {
                        string pass = pseudo_random_string();
                        string content = pseudo_random_string(100);
                        DataBlock *bp = new DataBlock(pass, content);
                        bp->completion_action(new ACT_Check(bp, content));
                        c.push(bp);
                }

                // process the communication queue, once if thread == false,
                // completely otherwise.
                if(thread) {
                        c.wait();
                        BOOST_CHECK_EQUAL(num_completions, loop_num);
                } else {
                        c();
                        BOOST_CHECK_EQUAL(num_completions, communicator_test_batch_size);
                }
        }
        
}



BOOST_AUTO_TEST_CASE(case_print_completion_one)
{
        check_completion(false);
}


BOOST_AUTO_TEST_CASE(case_print_completion_thread)
{
        check_completion(true);
}


