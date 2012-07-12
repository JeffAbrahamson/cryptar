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

                const string local_dir(filename_from_random_bits());
                const string remote_dir(filename_from_random_bits());
                const string remote_host(pseudo_random_string());
                const string crypto_key(pseudo_random_string());
                const StageType stage_type = stage_out_fs;
                const TransportType transport_type = rsync_push;

                const string filename("/tmp/cryptar-config-test-" + filename_from_random_bits());
                const string password(pseudo_random_string());

                Config c;
                c.local_dir(local_dir);
                c.remote_dir(remote_dir);
                c.remote_host(remote_host);
                c.crypto_key(crypto_key);
                c.stage_type(stage_type);
                c.transport_type(transport_type);
                c.save(filename, password);

                Config c2(filename, password);
                BOOST_CHECK(c2.local_dir() == local_dir);
                BOOST_CHECK(c2.remote_dir() == remote_dir);
                BOOST_CHECK(c2.remote_host() == remote_host);
                BOOST_CHECK(c2.crypto_key() == crypto_key);
                BOOST_CHECK(c2.stage_type() == stage_type);
                BOOST_CHECK(c2.transport_type() == transport_type);

                // FIXME: cleanup filename
        }
        
}



BOOST_AUTO_TEST_CASE(types)
{
        test_persist();
}

