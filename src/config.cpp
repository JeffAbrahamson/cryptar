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


#include <string>

#include "cryptar.h"

using namespace cryptar;
using namespace std;


/*
  For making new configs from scratch.
*/
Config::Config()
{
}


/*
  For making new configs from a stored config, probably in an
  encrypted file somewhere.
*/
Config::Config(const string &config_name)
{
}


string Config::staging_dir() const
{
        return string("/tmp/cryptar-") + getenv("HOME");
}


string Config::push_to_remote() const
{
        return string();
}


string Config::pull_from_remote() const
{
        return string();
}


