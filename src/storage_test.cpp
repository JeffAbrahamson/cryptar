/*
  Copyright 2013  Jeff Abrahamson
  
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
#include <string>

#include "cryptar.h"
#include "test_text.h"


using namespace cryptar;
using namespace std;


namespace {

        void file_storage(bool thread)
        {
                mode(Verbose, true);
                mode(Testing, true);
                mode(Threads, thread);

                string dir_name(temp_dir_name());
                FileStorageService fss(dir_name);
                for(int i = 0; i < 20; i++) {
                        string crypto_key(pseudo_random_string());
                        string contents(pseudo_random_string(200));
                        DataBlock *db = block_by_content<DataBlock>(crypto_key, contents);
                        fss.write(db);

                        DataBlock *db2 = block_by_id<DataBlock>(crypto_key, db->id());
                        fss.read(db2);

                        BOOST_CHECK_EQUAL(db2->plain_text(), contents);

                        fss.remove(db);
                }
                ::rmdir(dir_name.c_str());
        }

}


BOOST_AUTO_TEST_CASE(case_file_storage_one)
{
        file_storage(false);
}


BOOST_AUTO_TEST_CASE(case_file_storage_thread)
{
        file_storage(false);
}


