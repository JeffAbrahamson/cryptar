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



#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "cryptar.h"
#include "test_text.h"
#include "system.h"


using namespace cryptar;
using namespace std;



vector_string cryptar::test_text()
{
        vector_string text;
        text.push_back("");     // Yes, test the empty string
        text.push_back("This is the forest primeval.");
        text.push_back("This is the forest primeval."); // Test that a dup is ok
        text.push_back("This is the forest primeval."
                       "The murmuring pines and the hemlocks,\n"
                       "Bearded with moss, and in garments green, indistinct in the twilight,\n"
                       "Stand like Druids of eld, with voices sad and prophetic,\n"
                       "Stand like harpers hoar, with beards that rest on their bosoms."
                       "Loud from its rocky caverns, the deep-voiced neighboring ocean"
                       "Speaks, and in accents disconsolate answers the wail of the forest.");
        // This next also tests a high-ascii character.  Note that this file should be UTF-8.
        text.push_back("This is the forest primeval; but where are the hearts that beneath it"
                       "Leaped like the roe, when he hears in the woodland the voice of the huntsman?"
                       "Where is the thatch-roofed village, the home of Acadian farmers,--"
                       "Men whose lives glided on like rivers that water the woodlands,"
                       "Darkened by shadows of earth, but reflecting an image of heaven?"
                       "Waste are those pleasant farms, and the farmers forever departed!"
                       "Scattered like dust and leaves, when the mighty blasts of October"
                       "Seize them, and whirl them aloft, and sprinkle them far o'er the ocean"
                       "Naught but tradition remains of the beautiful village of Grand-Pré.");
        // Just finish the stanza
        text.push_back("Ye who believe in affection that hopes, and endures, and is patient,"
                       "Ye who believe in the beauty and strength of woman's devotion,"
                       "List to the mournful tradition, still sung by the pines of the forest;"
                       "List to a Tale of Love in Acadie, home of the happy.");
        // The empty string is still ok
        text.push_back("");
        // Test that a dup later is ok
        text.push_back("This is the forest primeval.");
        // Test some more characters outside low ascii
        text.push_back("François I était roi de la France."
                       "Une île, un mèl, eine übersetzung.");
        return text;
}


map<string, string> cryptar::orderly_text()
{
        map<string, string> text;
        text["A"] = "This is the forest primeval.";
        text["B"] = "The murmuring pines and the hemlocks,";
        text["C"] = "Bearded with moss, and in garments green, indistinct in the twilight,";
        text["D"] = "Stand like Druids of eld, with voices sad and prophetic,";
        text["E"] = "Stand like harpers hoar, with beards that rest on their bosoms.";
        text["F"] = "Loud from its rocky caverns, the deep-voiced neighboring ocean";
        text["G"] = "Speaks, and in accents disconsolate answers the wail of the forest.";
        text["H"] = "This is the forest primeval; but where are the hearts that beneath it";
        text["I"] = "Leaped like the roe, when he hears in the woodland the voice of the huntsman?";
        text["J"] = "Where is the thatch-roofed village, the home of Acadian farmers,--";
        text["K"] = "Men whose lives glided on like rivers that water the woodlands,";
        text["L"] = "Darkened by shadows of earth, but reflecting an image of heaven?";
        text["M"] = "Waste are those pleasant farms, and the farmers forever departed!";
        text["N"] = "Scattered like dust and leaves, when the mighty blasts of October";
        text["O"] = "Seize them, and whirl them aloft, and sprinkle them far o'er the ocean";
        text["P"] = "Naught but tradition remains of the beautiful village of Grand-Pré.";
        text["Q"] = "Ye who believe in affection that hopes, and endures, and is patient,";
        text["R"] = "Ye who believe in the beauty and strength of woman's devotion,";
        text["S"] = "List to the mournful tradition, still sung by the pines of the forest;";
        text["T"] = "List to a Tale of Love in Acadie, home of the happy.";

        return text;
}


/*
  In order to regression test phrase_to_key(), it's useful to know
  what we think return values should be.  This table is generated by
  printing values in phrase_to_key().
*/
map<string, pair<unsigned int, string> > cryptar::crypto_key_map()
{
        map<string, pair<unsigned int, string> > crypto_map;

        crypto_map.insert(pair<string, pair<unsigned int, string> >(string(""),
                                                           pair<unsigned int, string>(1600,
                                                                             string("qV2S1Nc5Y42g0flCVdLQDNLE2fLqZsR4on1HCPijVPY="))));
        crypto_map.insert(pair<string, pair<unsigned int, string> >(string("This is the forest primeval."),
                                                           pair<unsigned int, string>(1628,
                                                                             string("gfCxaDWWsfz9XrcC2anhaqiptfVSqO/hYxEzoS5aRfI="))));
        crypto_map.insert(pair<string, pair<unsigned int, string> >(string("This is the forest primeval.The murmuring pines and the hemlocks,\n"
                                                                  "Bearded with moss, and in garments green, indistinct in the twilight,\n"
                                                                  "Stand like Druids of eld, with voices sad and prophetic,\n"
                                                                  "Stand like harpers hoar, with beards that rest on their bosoms."
                                                                  "Loud from its rocky caverns, the deep-voiced neighboring ocean"
                                                                  "Speaks, and in accents disconsolate answers the wail of the forest."),
                                                           pair<unsigned int, string>(1985,
                                                                             string("G2f9r/A96BPFXlJEP9YYgEp6c8Lqpi8R1l9s+ybIKb4="))));
        crypto_map.insert(pair<string, pair<unsigned int, string> >(string("This is the forest primeval; but where are the hearts that beneath it"
                                                                  "Leaped like the roe, when he hears in the woodland the voice of the huntsman?"
                                                                  "Where is the thatch-roofed village, the home of Acadian farmers,--"
                                                                  "Men whose lives glided on like rivers that water the woodlands,"
                                                                  "Darkened by shadows of earth, but reflecting an image of heaven?"
                                                                  "Waste are those pleasant farms, and the farmers forever departed!"
                                                                  "Scattered like dust and leaves, when the mighty blasts of October"
                                                                  "Seize them, and whirl them aloft, and sprinkle them far o'er the ocean"
                                                                  "Naught but tradition remains of the beautiful village of Grand-Pré."),
                                                           pair<unsigned int, string>(2207,
                                                                             string("g/rkpRNqUidxrvw9DRMy8hWD3rpiJCp33e+iYxkEHTw="))));
        crypto_map.insert(pair<string, pair<unsigned int, string> >(string("Ye who believe in affection that hopes, and endures, and is patient,"
                                                                  "Ye who believe in the beauty and strength of woman's devotion,"
                                                                  "List to the mournful tradition, still sung by the pines of the forest;"
                                                                  "List to a Tale of Love in Acadie, home of the happy."),
                                                           pair<unsigned int, string>(1852,
                                                                             string("toSiLyJsQ/g9VIWZzpQuU0GiWl6YRk2nA1DKKyWnQaU="))));
        crypto_map.insert(pair<string, pair<unsigned int, string> >(string(""),
                                                           pair<unsigned int, string>(1600,
                                                                             string("qV2S1Nc5Y42g0flCVdLQDNLE2fLqZsR4on1HCPijVPY="))));
        crypto_map.insert(pair<string, pair<unsigned int, string> >(string("This is the forest primeval."),
                                                           pair<unsigned int, string>(1628,
                                                                             string("gfCxaDWWsfz9XrcC2anhaqiptfVSqO/hYxEzoS5aRfI="))));
        crypto_map.insert(pair<string, pair<unsigned int, string> >(string("François I était roi de la France.Une île, un mèl, eine übersetzung."),
                                                           pair<unsigned int, string>(1673,
                                                                             string("piRABvyiQ8ZNgTyHqP91h9n7Vfnti5E2S47RG9KnJV8="))));

        return crypto_map;
}



/*
  Provide the name of a temporary file that we can also use as a directory name.
  Which is why I'm not using mktemp() and its kin.
*/
string cryptar::temp_file_name() {
        return temp_file_name(std::string());
}



/*
  Provide the name of a temporary file that we can also use as a directory name.
  Create it within the provided directory.
*/
string cryptar::temp_file_name(const string &in_dir_name)
{
        ostringstream tmp_name_s;
        if(in_dir_name.empty()) {
                const char *logname = getenv("LOGNAME");
                tmp_name_s << "/tmp/cryptar-"
                           << (logname ? logname : "unknown")
                           << "-" << getpid() << "-" << time(0)
                           << filename_from_random_bits();
        } else {
                assert('/' == in_dir_name.back());
                tmp_name_s << in_dir_name << filename_from_random_bits();
        }
        string tmp_name = tmp_name_s.str();
        return tmp_name;
}


/*
  Provide the name of a temporary directory name.
  Also create the directory.
*/
string cryptar::temp_dir_name()
{
        string dir_name(temp_file_name() + "/");
        if(mkdir(dir_name.c_str(), 0700))
                throw_system_error("temp_dir_name()");
        // FIXME    (Error should indicate what happened and what name)
        // FIXME    (The use of mkdir is repeated in the code, it should be done once)
        return dir_name;
}



void cryptar::clean_temp_dir(string &in_dir_name)
{
        string command("rm -rf " + in_dir_name);
        int ret = system(command.c_str());
        if(-1 == ret)
                perror("  Failed to remove temp dir");
        else if(0 != ret)
                cout << "Removed temp dir but with non-zero return value: " << ret << endl;
}

