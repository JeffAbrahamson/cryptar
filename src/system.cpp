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
#include <string.h>

#include "system.h"


using namespace cryptar;
using namespace std;


/*
  Signal a fatal error from some system function.
  Mostly, this is for filesystem errors.

  FIXME    This is a candidate for being in a general utility area.
*/
void cryptar::throw_system_error(const string &label)
{
        const size_t len = 1024; // arbitrary
        char errstr[len];
        int errnum = errno;
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
        strerror_r(errnum, errstr, len);
        cerr << label << " error: " << errstr << endl;
#else
        // I'm not quite clear why the man page wants us to
        // pass errstr and len, but GNU libc clearly puts the
        // error message in the returned char*.
        char *errstr_r = strerror_r(errnum, errstr, len);
        cerr << label << " error: " << errstr_r << endl;
#endif
        throw(label);
}



