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
#define BOOST_TEST_MODULE crypt_tests
#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <pstreams/pstream.h>
#include <string>

#include "cryptar.h"
#include "test_text.h"


using namespace cryptar;
using namespace std;

namespace {

        void test_message_digest(const string &message)
        {
                // I don't believe I will see a bug related to having a
                // single-quote in a string, but the test is easy to write if
                // I assume I don't.  So replace ' with $ in the input string.
                string safe_message;
                replace_copy(message.begin(), message.end(),
                             back_inserter(safe_message), '\'', '$');
        
                string msg_digest = message_digest(safe_message, false);

                string command("echo -n '");
                command.append(safe_message);
                command.append("' | openssl dgst -sha256 -binary | openssl base64 -e");
                redi::ipstream proc(command.c_str());
                string openssl_digest;
                proc >> openssl_digest;

                BOOST_CHECK_MESSAGE(msg_digest == openssl_digest, safe_message);
        }
        

        /*
          Return the number of errors that occur.  Return true if an
          error occurs, false otherwise.
        */
        void test_encryption(const string &message)
        {
                string password(message_digest(message, false));
                string cipher_text = encrypt(message, password);
                string plain_text = decrypt(cipher_text, password);
                BOOST_CHECK_MESSAGE(cipher_text != message,
                                    "\"" << message << "\"" <<
                                    "Encryption did not change message!");
                BOOST_CHECK(message == plain_text);
        }



        /*
          Check that message_digest() and pseudo_random_string()
          return reasonable values.
        */
        void test_lengths()
        {
                const int N = 5000;
                int prs_error_count = 0;
                int md_error_count = 0;
                int mdfs_error_count = 0;
                srand((int)time(0));
                unsigned int n;
                cout << "  [begin length test]" << endl;
                time_t start_time = time(0);
                assert(start_time > 0);
                for(int i = 0; i < N; i++) {
                        n = rand() & 0xFFFF; // Else we take too long
                        string s(pseudo_random_string(n));
                        if(s.size() != n)
                                prs_error_count++;
                        string md(message_digest(s));
                        if(md.size() != 44)
                                md_error_count++;
                        string mdfs(message_digest(s, true));
                        if(mdfs.size() != 44)
                                mdfs_error_count++;
                        if(mdfs.find('/') != string::npos)
                                mdfs_error_count++;
                }
                time_t end_time = time(0);
                BOOST_CHECK(end_time > 0);
                cout << "  ...done in " << end_time - start_time << " seconds." << endl;

                BOOST_CHECK(0 == prs_error_count); // pseudo-random string
                BOOST_CHECK(0 == md_error_count);  // message digest(,false)
                BOOST_CHECK(0 == mdfs_error_count); // message_digest(,true)
        }        
}



BOOST_FIXTURE_TEST_SUITE(crypt_tests, Messages)

BOOST_AUTO_TEST_CASE(message_digest)
{
        test(test_message_digest);
}


BOOST_AUTO_TEST_CASE(encryption)
{
        test(test_encryption);
}


BOOST_AUTO_TEST_CASE(lengths)
{
        test_lengths();
}

BOOST_AUTO_TEST_SUITE_END()
