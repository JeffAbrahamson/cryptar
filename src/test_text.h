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



#ifndef __TEST_TEXT_H__
#define __TEST_TEXT_H__ 1


#include <boost/test/unit_test.hpp>
#include <map>
#include <vector>
#include <string>

#include "cryptar.h"


namespace cryptar {

        typedef std::vector<std::string> vector_string;

        cryptar::vector_string test_text();
        std::map<std::string, std::string> orderly_text();
        std::string temp_file_name();
        
        struct Messages {

                vector_string messages;
        
                Messages()
                {
                        mode(Verbose, false);
                        mode(Testing, true);

                        BOOST_TEST_MESSAGE("Instantiating messages");
                        messages = test_text();
                }
        
                ~Messages()
                {
                        BOOST_TEST_MESSAGE("Deallocating messages");
                        // And deallocation happens automatically.
                }


                /*
                  Call a function on each message.  To be useful, that
                  function should test something.
                */
                void test(void(func)(const std::string &s))
                {
                        for_each(messages.begin(), messages.end(), func);
                }
        };

        
}

#endif  /* __TEST_TEXT_H__*/
