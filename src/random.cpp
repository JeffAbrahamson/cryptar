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


#include <iostream>

#include "cryptar.h"


using namespace std;
using namespace cryptar;


/*
  The only point is to output what we think should be different on
  every run.  We'll check that it is from another process.
*/
int main(int argc, char *argv[])
{
        for(int i = 0; i < 50; ++i)
                cout << filename_from_random_bits() << endl;
}

