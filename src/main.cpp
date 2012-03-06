/*
  Copyright 2012  Jeff Abrahamson
  
  This file is part of cryptar.
  
  This software might be free, it might be commercial.
  You may safely assume that using it for personal, non-commercial
  use will remain acceptable.  The eventual license may, however, be a
  split free/commercial license, such as the one long used by
  Sleepycat.

  In any case, this software is explicitly licensed the terms of the
  GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any
  later version.
  
  cryptar is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with cryptar.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <boost/program_options.hpp>
//#include <fstream>
//#include <iomanip>
//#include <iostream>
#include <stdexcept>
#include <stdio.h>
//#include <stdlib.h>
//#include <sys/types.h>
#include <termios.h>
//#include <unistd.h>

#include "cryptar.h"


namespace BPO = boost::program_options;
using namespace cryptar;
using namespace std;


namespace {

        BPO::variables_map parse_options(int, char *[]);
        string get_password(const string prompt = "Password:  ");
        bool change_password(const string);


        class help_exception : public exception {};


        BPO::variables_map parse_options(int argc, char *argv[])
        {
                BPO::options_description general("General options");
                general.add_options()
                        ("help,h",
                         "Produce help message")
                        ("verbose,v",
                         "Emit debugging information")
                        ("action,a", BPO::value<string>(),
                         "One of create, backup, restore, backup-daemon, multi-backup-daemon")
                        ("name,n", BPO::value<string>(),
                         "Name of archive on which to act");

                BPO::options_description create("Archive creation");
                create.add_options()
                        ("user,U", BPO::value<string>(),
                         "User name on remote host")
                        ("password,P", BPO::value<string>(),
                         "Encryption password for remote store (ill-advised except for testing)")
                        ("host,H", BPO::value<string>(),
                         "Remote host name")
                        ("root,R", BPO::value<string>(),
                         "Absolute path to backup");

                BPO::options_description backup_restore("Backup and Restore");
                backup_restore.add_options()
                        ("begins,b", BPO::value<string>(),
                         "Backup or restore only paths beginning with string")
                        ("ends,e", BPO::value<string>(),
                         "Backup or restore only paths ending with string")
                        ("contains,c", BPO::value<string>(),
                         "Backup or restore only paths containing string");

                BPO::options_description restore("Restore only");
                restore.add_options()
                        ("restore-to", BPO::value<string>(),
                         "Restore to this directory (default in place)");

                BPO::options_description options("Allowed options");
                options.add(general).add(create).add(backup_restore).add(restore);
        
                BPO::positional_options_description pos;
                pos.add("match-key", -1);
        
                BPO::variables_map opt_map;
                BPO::store(BPO::command_line_parser(argc, argv).options(options).positional(pos).run(),
                           opt_map);
                BPO::notify(opt_map);

                if(opt_map.count("help")) {
                        cout << options << endl;
                        throw help_exception();
                }
        
                return opt_map;
        }



        string get_password(const string prompt)
        {
                cout << prompt;
        
                // Get pass phrase without echoing it
                termios before, after;
                tcgetattr(STDIN_FILENO, &before);
                after = before;
                after.c_lflag &= (~ICANON); // Disable canonical mode, including line buffering
                after.c_lflag &= (~ECHO);   // Don't echo characters
                tcsetattr(STDIN_FILENO, TCSANOW, &after);

                const int pass_len = 10240; // arbitrary, hopefully long enough
                char pass_phrase[pass_len];
                cin.getline(pass_phrase, pass_len);

                tcsetattr(STDIN_FILENO, TCSANOW, &before);
                cout << endl;

                // Iterate hash (arbitrarily) 50 times.  Motivated by gpg's behavior.
                string digest(pass_phrase);
                for(int i = 0; i < 50; i++)
                        digest = message_digest(digest, false);
        
                // Clear original pass phrase to minimize risk of seeing it in a swap or core image
                bzero(pass_phrase, sizeof(pass_len));
        
                return digest;
        }


}



/*
  Do that thing that we do.
*/
int main(int argc, char *argv[])
{
        BPO::variables_map options;
        try {
                options = parse_options(argc, argv);
        }
        catch(help_exception) {
                return 0;       // User asked for help, done
        }
        catch(exception& e) {
                // Something went wrong.  Say so and exit with error.
                cerr << e.what() << endl;
                // Options aren't available, so be verbose to be clear.
                cerr << "(Error is fatal, quitting before doing anything.)" << endl;
                return 1;
        }

        const bool verbose = options.count("verbose") > 0;
        mode(Verbose, verbose);

        bool is_test = options.count("TEST") > 0;
        mode(Testing, is_test);

        string passwd;
        if(is_test)
                passwd = options["TEST"].as<string>();
        else
                passwd = get_password();
        
        // do something
        if(options.count("passwd")) {
                //change_password(passwd);
                return 0;
        }

        Communicator send;
        Communicator receive;
        return 0;
}
