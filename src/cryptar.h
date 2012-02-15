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



#ifndef __CRYPTAR_H__
#define __CRYPTAR_H__ 1

namespace cryptar {

        /* ************************************************************ */
        /* Compression */

        std::string compress(const std::string &);
        std::string decompress(const std::string &, unsigned int = 0);


        /* ************************************************************ */
        /* Encryption */
        
        // Compute a hash (message digest).
        std::string message_digest(const std::string &message,
                                   const bool filesystem_safe = false);

        // Return a (not necessarily human readable) string of random bits.
        std::string pseudo_random_string(int length);
        
        // Crypto++ is documented at http://www.cryptopp.com/
        // and http://www.cryptopp.com/fom-serve/cache/1.html
        // and http://www.cryptopp.com/wiki/FAQ

        /*
          This is a simple mixin class that provides encryption and decryption.
        */
        std::string encrypt(const std::string &plain_message,
                            const std::string &password);
        std::string decrypt(const std::string &cipher_message,
                            const std::string &password);


        /* ************************************************************ */
        /* mode */

        /*
          Maintain a map of modes.
          
          Written for verbose and testing, to avoid having to pass them around
          everywhere in case we need them deep down.  Especially verbose.
  
          Some might call this a kludge or even inelegant.  Abused, it would be.
        */


        enum Mode {
                Verbose,
                Testing,
        };

        void mode(const Mode m, const bool new_state);
        const bool mode(const Mode m);


}

#endif  /* __CRYPTAR_H__*/
