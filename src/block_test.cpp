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
#include <sys/stat.h>
#include <sys/types.h>

#include "cryptar.h"
#include "system.h"
#include "test_text.h"


using namespace cryptar;
using namespace std;


namespace {

        /*
          Check that BlockId's behave as we expect.

          (That is, like strings, comparable as such, but generating
          their own unique id's.)
        */
        void check_block_id()
        {
                cout << "check_block_id()" << endl;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, false);

                cout << "check_block_id()" << endl;
                BlockId b1, b2;
                BOOST_CHECK(b1 != b2);
                BOOST_CHECK(!(b1 == b2));
                BlockId b3(b1);
                BOOST_CHECK(b1 == b3);
                BOOST_CHECK(!(b1 != b3));
        }

        
        /*
          Verify that a data block can encrypt and decrypt again.
        */
        void check_data_block()
        {
                cout << "check_data_block()" << endl;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, false);

                ConfigParam params(no_transport);
                params.m_passphrase = pseudo_random_string();
                string content = pseudo_random_string(100);
                DataBlock *bp = block_by_content<DataBlock>(params.transport(),
                                                            params.m_passphrase,
                                                            content);
                BOOST_CHECK_EQUAL(content, bp->plain_text());
        }


        /*
          Serialisation test.
        */
        void check_serialise()
        {
                cout << "check_serialise()" << endl;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, false);
                
                ConfigParam params(no_transport);
                params.m_passphrase = pseudo_random_string();
                const string content(pseudo_random_string(100));
                DataBlock *bp = block_by_content<DataBlock>(params.transport(),
                                                            params.m_passphrase,
                                                            content);
                BOOST_CHECK_EQUAL(content, bp->plain_text());

                DataBlock *bp2 = block_empty<DataBlock>(params.transport(),
                                                        params.m_passphrase);
                bp2->from_stream(bp->to_stream());
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
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, thread);
                
                ConfigParam params(fs);
                params.m_passphrase = pseudo_random_string();
                params.m_local_dir = temp_dir_name();
                shared_ptr<Config> config = make_config(params);

                // Start with 1 so that we can verify that ACT's have
                // been initialized.
                const int loop_num = 10;
                for(int i = 1; i <= loop_num; i++) {
                        string pass = pseudo_random_string();
                        string content = pseudo_random_string(100);
                        DataBlock *bp = block_by_content<DataBlock>(params.transport(),
                                                                    params.m_passphrase,
                                                                    content);
                        bp->completion_action(new ACT_Check(bp, content));
                        bp->write();
                }

                // process the communication queue, once if thread == false,
                // completely otherwise.
                if(thread) {
                        //comm_send->wait();
                        BOOST_CHECK_EQUAL(num_completions, loop_num);
                } else {
                        ;
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

                ConfigParam params(fs);
                params.m_passphrase = pseudo_random_string();
                params.m_local_dir = temp_dir_name();

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
                        shared_ptr<Config> config = make_config(params);

                        // Start with 1 so that we can verify that ACT's have
                        // been initialized.
                        for(int i = 1; i <= loop_num; i++) {
                                string pass = pseudo_random_string();
                                string content = pseudo_random_string(100);
                                DataBlock *bp = block_by_content<DataBlock>(params.transport(),
                                                                            pass,
                                                                            content);
                                bp->completion_action(new ACT_Check(bp, content));
                                bp->write();
                                blocks.push_back(BlockNote(bp->id(), pass, content));
                        }

                        // process the communication queue, once if thread == false,
                        // completely otherwise.
                        if(thread) {
                                //c_send->wait();
                                BOOST_CHECK_EQUAL(num_completions, loop_num);
                        } else {
                                ;
                        }
                }

                // Now fetch those same blocks from the staging area.
                num_completions = 0;
                shared_ptr<Config> config = make_config(params);

                for(auto it = blocks.begin(); it != blocks.end(); ++it) {
                        DataBlock *bp = block_by_id<DataBlock>(params.transport(),
                                                               it->m_pass,
                                                               it->m_id);
                        bp->completion_action(new ACT_Check(bp, it->m_content));
                        bp->read();
                }
                // process the communication queue, once if thread == false,
                // completely otherwise.
                if(thread) {
                        //c_recv->wait();
                        BOOST_CHECK_EQUAL(num_completions, loop_num);
                } else {
                        ;
                }
        }
}


BOOST_AUTO_TEST_CASE(block_id)
{
        check_block_id();
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

#if 0
BOOST_AUTO_TEST_CASE(case_print_completion_thread)
{
        check_completion(true);
}
#endif

BOOST_AUTO_TEST_CASE(case_print_staging_one)
{
        check_staging(false);
}

#if 0
BOOST_AUTO_TEST_CASE(case_print_staging_thread)
{
        check_staging(true);
}
#endif

