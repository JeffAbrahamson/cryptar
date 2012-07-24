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
#include <map>
#include <string>
#include <utility>

#include "cryptar.h"
#include "test_text.h"


using namespace cryptar;
using namespace std;


namespace {

        /*
          Verify that a data block can encrypt and decrypt again.
        */
        void check_data_block()
        {
                cout << "check_data_block()" << endl;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, false);
                
                string pass = pseudo_random_string();
                string content = pseudo_random_string(100);
                DataBlock *bp = block_by_content<DataBlock>(pass, content);
                BOOST_CHECK_EQUAL(content, bp->plain_text());
        }



        /*
          Serialisation test.  Just serialise a block to a file and restore it.
        */
        void check_serialise()
        {
                cout << "check_serialise()" << endl;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, false);
                
                string pass = pseudo_random_string();
                string content = pseudo_random_string(100);
                DataBlock *bp = block_by_content<DataBlock>(pass, content);
                const string staging_dir(temp_file_name());
                StageOutFS stage(staging_dir); // causes staging_dir to be created
                bp->write(staging_dir);
                const BlockId id = bp->id();
                BOOST_CHECK_EQUAL(content, bp->plain_text());

                DataBlock *bp2 = block_by_id<DataBlock>(pass, id);
                bp2->read(staging_dir);
                BOOST_CHECK_EQUAL(content, bp2->plain_text());
        }
        

                
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
                cout << "check_completion(" << thread << ")" << endl;
                num_completions = 0;
                //mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, thread);
                Communicator c(new NoStage(), new Transport(Config()));

                // Start with 1 so that we can verify that ACT's have
                // been initialized.
                const int loop_num = 10;
                for(int i = 1; i <= loop_num; i++) {
                        string pass = pseudo_random_string();
                        string content = pseudo_random_string(100);
                        DataBlock *bp = block_by_content<DataBlock>(pass, content);
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
                        const int expected_num = min(loop_num, communicator_test_batch_size);
                        BOOST_CHECK_EQUAL(num_completions, expected_num);
                }
        }


        /*
          Queue some blocks for transfer.  Stage them to files and delete the blocks.
          Queue some blocks for transfer, recover from staged.
          Their completion methods check that encryption is working.
        */
        void check_staging(bool thread)
        {
                cout << "check_staging(" << thread << ")" << endl;
                num_completions = 0;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, thread);
                const string staging_dir(temp_file_name());

                struct BlockNote {
                        BlockNote(const BlockId &in_id,
                                  const string &in_pass,
                                  const string &in_content)
                                : m_id(in_id), m_pass(in_pass), m_content(in_content) {};
                        BlockId m_id;
                        string m_pass;
                        string m_content; // plain-text
                };
                vector<BlockNote> blocks;
                const int loop_num = 10;
                {
                        Communicator c_send(new StageOutFS(staging_dir),
                                            new Transport(Config()));
                        // Start with 1 so that we can verify that ACT's have
                        // been initialized.
                        for(int i = 1; i <= loop_num; i++) {
                                string pass = pseudo_random_string();
                                string content = pseudo_random_string(100);
                                DataBlock *bp = block_by_content<DataBlock>(pass, content);
                                bp->completion_action(new ACT_Check(bp, content));
                                c_send.push(bp);
                                blocks.push_back(BlockNote(bp->id(), pass, content));
                        }

                        // process the communication queue, once if thread == false,
                        // completely otherwise.
                        if(thread) {
                                c_send.wait();
                                BOOST_CHECK_EQUAL(num_completions, loop_num);
                        } else {
                                c_send();
                                const int expected_num = min(loop_num, communicator_test_batch_size);
                                BOOST_CHECK_EQUAL(num_completions, expected_num);
                        }
                }

                // Now fetch those same blocks from the staging area.
                num_completions = 0;
                Communicator c_recv(new StageInFS(staging_dir),
                                    new Transport(Config()));
                for(auto it = blocks.begin(); it != blocks.end(); ++it) {
                        DataBlock *bp = block_by_id<DataBlock>(it->m_pass, it->m_id);
                        bp->completion_action(new ACT_Check(bp, it->m_content));
                        c_recv.push(bp);
                }
                // process the communication queue, once if thread == false,
                // completely otherwise.
                if(thread) {
                        c_recv.wait();
                        BOOST_CHECK_EQUAL(num_completions, loop_num);
                } else {
                        c_recv();
                        const int expected_num = min(loop_num, communicator_test_batch_size);
                        BOOST_CHECK_EQUAL(num_completions, expected_num);
                }
        }
}


BOOST_AUTO_TEST_CASE(case_data_block)
{
        check_data_block();
}

BOOST_AUTO_TEST_CASE(case_serialisation)
{
        check_serialise();
}

BOOST_AUTO_TEST_CASE(case_print_completion_one)
{
        check_completion(false);
}

BOOST_AUTO_TEST_CASE(case_print_completion_thread)
{
        check_completion(true);
}

BOOST_AUTO_TEST_CASE(case_print_staging_one)
{
        check_staging(false);
}

BOOST_AUTO_TEST_CASE(case_print_staging_thread)
{
        check_staging(true);
}

