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

        void test_types()
        {
                {
                        Stage s;
                        BOOST_CHECK(base_stage == s.stage_type());
                }
                
                {
                        StageOutFS s(".");
                        BOOST_CHECK(stage_out_fs == s.stage_type());
                }

                {
                        StageInFS s(".");
                        BOOST_CHECK(stage_in_fs == s.stage_type());
                }
        }
        
}



BOOST_AUTO_TEST_CASE(types)
{
        cout << "  [types]" << endl;
        test_types();
}

