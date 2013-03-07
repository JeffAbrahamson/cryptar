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
#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <pstreams/pstream.h>
#include <string>

#include "cryptar.h"
#include "test_text.h"


using namespace cryptar;
using namespace std;


namespace {
        
        void test_persist()
        {
                cout << "  [persist]" << endl;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, false);

                ConfigParam params;
                params.m_local_dir = "/tmp/cryptar-config-test-" + filename_from_random_bits();
                params.m_remote_dir = filename_from_random_bits();
                params.m_remote_host = pseudo_random_string();
                params.m_passphrase = pseudo_random_string();
                params.m_transport_type = fs;

                const string filename("/tmp/cryptar-config-test-" + filename_from_random_bits());

                shared_ptr<Config> c = make_config(params);
                BOOST_CHECK_EQUAL(params.m_local_dir, c->local_dir());
                BOOST_CHECK_EQUAL(params.m_remote_dir, c->remote_dir());
                BOOST_CHECK_EQUAL(params.m_remote_host, c->remote_host());
                // We don't otherwise reveal transport_type, so don't test it
                //BOOST_CHECK_EQUAL(params.m_transport_type, c->transport_type());
                c->save(filename, params.m_passphrase);

                shared_ptr<Config> c2 = make_config(filename, params.m_passphrase);
                BOOST_CHECK_EQUAL(params.m_local_dir, c2->local_dir());
                BOOST_CHECK_EQUAL(params.m_remote_dir, c2->remote_dir());
                BOOST_CHECK_EQUAL(params.m_remote_host, c2->remote_host());
                // We don't otherwise reveal transport_type, so don't test it
                //BOOST_CHECK_EQUAL(params.m_transport_type, c2->transport_type());
                // We don't otherwise reveal crypto_key(), so don't test it
                //BOOST_CHECK_EQUAL(c->crypto_key(), c2->crypto_key);

                // FIXME: cleanup filename
        }


#if 0
        /*
          Store data using the db interface, then recover the data.
        */
        void test_end_to_end(bool threaded)
        {
                cout << "  [db_end_to_end (" << (threaded ? "threaded" : "single-threaded") << ")]" << endl;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, threaded);
                
                // FIXME: Why  is this test here?  Should  it be here?
                // Finish it somewhere, but probably in db_test.cpp.
        }
#endif   
}


BOOST_AUTO_TEST_CASE(persist)
{
        test_persist();
}


#if 0
BOOST_AUTO_TEST_CASE(db_end_to_end)
{
        test_end_to_end(false);
        test_end_to_end(true);
}
#endif

