/*
  Copyright 2012  Jeff Abrahamson
  
  This file is part of crytpar.
  
  crytpar is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  crytpar is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with crytpar.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "cryptar.h"


using namespace cryptar;
using namespace std;


Transport::Transport(const Config &in_config)
        : m_config(in_config)
{
}


/*
TransportRsyncPush::TransportRsyncPush(const Config &in_config)
{
}
*/
