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


using namespace cryptar;
using namespace std;


namespace {

        // Tests to do:
        // - root can persist itself and be fetched
        // - add databases and fetch them
        // - check that added db's are present, and that those not added are not

        void check_root()
        {
                cout << "check_root()" << endl;
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, false);
                
        }

}

BOOST_AUTO_TEST_CASE(root)
{
        check_root();
}


