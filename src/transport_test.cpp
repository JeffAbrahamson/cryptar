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
#include <memory>
#include <pstreams/pstream.h>
#include <string>

#include "cryptar.h"
#include "test_text.h"


using namespace cryptar;
using namespace std;


namespace {

        void test_types()
        {
                string passphrase = pseudo_random_string();
                shared_ptr<Config> c = make_config(passphrase);

                /* We'll make transport objects directly (to make sure
                   the base class is correctly polymorphic) and
                   through the dedicated factory functions.
                */
                Transport *t1 = make_transport(no_transport, c);
                BOOST_CHECK(no_transport == t1->transport_type());
                NoTransport *mt1 = make_no_transport(c);
                BOOST_CHECK(no_transport == mt1->transport_type());
                
                Transport *t2 = make_transport(fs_out, c);
                BOOST_CHECK(fs_out == t2->transport_type());
                TransportFSOut *mt2 = make_transport_fsout(c);
                BOOST_CHECK(fs_out == mt2->transport_type());

                Transport *t3 = make_transport(fs_in, c);
                BOOST_CHECK(fs_in == t3->transport_type());
                TransportFSIn *mt3 = make_transport_fsin(c);
                BOOST_CHECK(fs_in == mt3->transport_type());
        }
        
}



BOOST_AUTO_TEST_CASE(types)
{
        cout << "  [types]" << endl;
        test_types();
}

