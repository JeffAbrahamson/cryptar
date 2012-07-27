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
                Communicator c(new NoStage(), new NoTransport(Config()));
                string pass = ""; // doesn't matter here

                // Start with 1 so that we can verify that ACT's have
                // been initialized.
                for(int i = 1; i <= 10; i++) {
                        //Block *bp = new Block(pass);
                        Block *bp = block_empty<Block>(pass);
                        bp->completion_action(new ACT_Print(i));
                        c.push(bp);
                }

                // process the communication queue, once if thread == false,
                // completely otherwise.
                if(thread)
                        c.wait();
                else
                        c();
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


