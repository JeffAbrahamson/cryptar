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
        

#if FIXME
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
                Config config("");
                config.transport_type(no_transport);
                shared_ptr<Communicator> c = config.sender();

                // Start with 1 so that we can verify that ACT's have
                // been initialized.
                const int loop_num = 10;
                for(int i = 1; i <= loop_num; i++) {
                        string pass = pseudo_random_string();
                        string content = pseudo_random_string(100);
                        DataBlock *bp = block_by_content<DataBlock>(pass, content);
                        bp->completion_action(new ACT_Check(bp, content));
                        c->push(bp);
                }

                // process the communication queue, once if thread == false,
                // completely otherwise.
                if(thread) {
                        c->wait();
                        BOOST_CHECK_EQUAL(num_completions, loop_num);
                } else {
                        (*c)();
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
                        Config config("");
                        config.local_dir(staging_dir);
                        config.transport_type(no_transport);
                        shared_ptr<Communicator> c_send = config.sender();

                        // Start with 1 so that we can verify that ACT's have
                        // been initialized.
                        for(int i = 1; i <= loop_num; i++) {
                                string pass = pseudo_random_string();
                                string content = pseudo_random_string(100);
                                DataBlock *bp = block_by_content<DataBlock>(pass, content);
                                bp->completion_action(new ACT_Check(bp, content));
                                c_send->push(bp);
                                blocks.push_back(BlockNote(bp->id(), pass, content));
                        }

                        // process the communication queue, once if thread == false,
                        // completely otherwise.
                        if(thread) {
                                c_send->wait();
                                BOOST_CHECK_EQUAL(num_completions, loop_num);
                        } else {
                                (*c_send)();
                                const int expected_num = min(loop_num, communicator_test_batch_size);
                                BOOST_CHECK_EQUAL(num_completions, expected_num);
                        }
                }

                // Now fetch those same blocks from the staging area.
                num_completions = 0;
                Config config("");
                config.local_dir(staging_dir);
                config.transport_type(no_transport);
                shared_ptr<Communicator> c_recv = config.receiver();

                for(auto it = blocks.begin(); it != blocks.end(); ++it) {
                        DataBlock *bp = block_by_id<DataBlock>(it->m_pass, it->m_id);
                        bp->completion_action(new ACT_Check(bp, it->m_content));
                        c_recv->push(bp);
                }
                // process the communication queue, once if thread == false,
                // completely otherwise.
                if(thread) {
                        c_recv->wait();
                        BOOST_CHECK_EQUAL(num_completions, loop_num);
                } else {
                        (*c_recv)();
                        const int expected_num = min(loop_num, communicator_test_batch_size);
                        BOOST_CHECK_EQUAL(num_completions, expected_num);
                }
        }
#endif
}

#if FIXME
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
#endif

