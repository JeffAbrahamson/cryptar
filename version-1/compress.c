/*  compress.c
 *  Copyright (C) 2002, 2003 by:
 *  Jeff Abrahamson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#include "compress.h"
#include "encryption.h"
#include "ios.h"
#include "protocol.h"


static void do_crypt_test(IOSBuf *ios_in);



IOSBuf *do_compress(IOSBuf *ios_in)
{
#ifndef CLEARTEXT
        void *data;
        unsigned long length;
        IOSBuf *ios_out;
        int ret;
        
        g_assert(ios_in);
        /* zlib could increase size by as much as 0.1% + 12 bytes */
        length = (float)ios_buffer_size(ios_in) * 1.001 + 13;
        data = g_malloc(length);
        if((ret = compress(data, &length, ios_buffer(ios_in),
                           ios_buffer_size(ios_in)))) {
                /* Could we handle this better? */
                g_warning("Compression failed:");
                if(ret == Z_MEM_ERROR)
                        g_warning("Z_MEM_ERROR: not enough memory");
                if(ret == Z_BUF_ERROR)
                        g_warning("Z_BUF_ERROR: not enough space in output buffer");
                exit(1);
        }
        ios_out = ios_new();
        ios_set_buffer(ios_out, data, length);
#else
        IOSBuf *ios_out; ios_out = ios_new_copy(ios_in);
#endif
        return ios_out;
}



IOSBuf *do_decompress(IOSBuf *ios_in, guint32 orig_len)
{
#ifndef CLEARTEXT
        void *data;
        unsigned long length;
        IOSBuf *ios_out;
        int ret;
        
        g_assert(ios_in);
        g_assert(orig_len > 0);
        length = orig_len;
        data = g_malloc(length);
        if((ret = uncompress(data, &length, ios_buffer(ios_in),
                             ios_buffer_size(ios_in)))) {
                g_warning("Decompression error");
                if(ret == Z_MEM_ERROR)
                        g_warning("Z_MEM_ERROR: "
                                "not enough memory");
                if(ret == Z_BUF_ERROR)
                        g_warning("Z_BUF_ERROR: "
                                "not enough space in output buffer");
                exit(1);
        }
        if(length != orig_len)
                g_warning("Decompression yielded an unexpected length.");
        ios_out = ios_new();
        ios_set_buffer(ios_out, data, length);
#else
        IOSBuf *ios_out; ios_out = ios_new_copy(ios_in); orig_len = 0;
#endif
        return ios_out;
}



/* Drive the compression/encryption test.
 */
void crypt_test()
{
        IOSBuf *ios;

        g_message("Lightly testing compression and encryption.");
        
        /* Probably better to generate some random text, but for now
           some Longfellow will have to suffice.
         */
        ios = ios_new();
        ios_append_string(ios, "This is the forest primeval. The murmuring pines and the hemlocks, Bearded with moss, and in garments green, indistinct in the twilight, Stand like Druids of eld, with voices sad and prophetic, Stand like harpers hoar, with beards that rest on their bosoms.  Loud from its rocky caverns, the deep-voiced neighboring ocean Speaks, and in accents disconsolate answers the wail of the forest.\n\nThis is the forest primeval; but where are the hearts that beneath it Leaped like the roe, when he hears in the woodland the voice of the huntsman? Where is the thatch-roofed village, the home of Acadian farmers -- Men whose lives glided on like rivers that water the woodlands, Darkened by shadows of earth, but reflecting an image of heaven?  Waste are those pleasant farms, and the farmers forever departed!  Scattered like dust and leaves, when the mighty blasts of October Seize them, and whirl them aloft, and sprinkle them far o'er the ocean.  Naught but tradition remains of the beautiful village of Grand-Pré.\n\nYe who believe in affection that hopes, and endures, and is patient, Ye who believe in the beauty and strength of woman's devotion, List to the mournful tradition still sung by the pines of the forest; List to a Tale of Love in Acadie, home of the happy.\n");
        do_crypt_test(ios);

        ios_reset(ios);
        ios_append_string(ios, "In the Acadian land, on the shores of the Basin of Minas, Distant, secluded, still, the little village of Grand-Pré Lay in the fruitful valley.  Vast meadows stretched to the eastward, Giving the village its name, and pasture to flocks without number. Dikes, that the hands of the farmers had raised with labor incessant, Shut out the turbulent tides; but at stated seasons the flood-gates Opened, and welcomed the sea to wander at will o'er the meadows. West and south there were fields of flax, and orchards and cornfields Spreading afar and unfenced o'er the plain; and away to the northward Blomidon rose, and the forests old, and aloft on the mountains Sea-fogs pitched their tents, and mists from the mighty Atlantic Looked on the happy valley, but ne'er from their station descended. There, in the midst of its farms, reposed the Acadian village. Strongly built were the houses, with frames of oak and of chestnut, Such as the peasants of Normandy built in the reign of the Henries. Thatched were the roofs, with dormer-windows; and gables projecting Over the basement below protected and shaded the door-way. There in the tranquil evenings of summer, when brightly the sunset Lighted the village street, and gilded the vanes on the chimneys,  Matrons and maidens sat in snow-white caps and in kirtles Scarlet and blue and green, with distaffs spinning the golden Flax for the gossiping looms, whose noisy shuttles within doors Mingled their sound with the whir of the wheels and the songs of the maidens. Solemnly down the street came the parish priest, and the children Paused in their play to kiss the hand he extended to bless them. Reverend walked he among them; and up rose matrons and maidens, Hailing his slow approach with words of affectionate welcome. Then came the laborers home from the field, and serenely the sun sank Down to his rest, and twilight prevailed.  Anon from the belfry Softly the Angelus sounded, and over the roofs of the village Columns of pale blue smoke, like clouds of incense ascending, Rose from a hundred hearths, the homes of peace and contentment. Thus dwelt together in love these simple Acadian farmers,-- Dwelt in the love of God and of man.  Alike were they free from Fear, that reigns with the tyrant, and envy, the vice of republics. Neither locks had they to their doors, nor bars to their windows; But their dwellings were open as day and the hearts of the owners; There the richest was poor, and the poorest lived in abundance.\n");
        do_crypt_test(ios);
        
        return;
}




/* Compress and encrypt some text, then decrypt and decompress it.
   Report if the input test differs from the output text.
 */
static void do_crypt_test(IOSBuf *ios_in)
{
        IOSBuf *ios_compressed, *ios_encrypted, *ios_decrypted, *ios_decompressed;
        int len;
       
			  /* Note this uses the default random key generated by the init_crypto()
				 * function, that is unless we generate another key somewhere else. */	
        g_assert(ios_in);
        len = ios_buffer_size(ios_in);
        ios_compressed = do_compress(ios_in);
        ios_encrypted = encrypt(ios_compressed);
        ios_decrypted = decrypt(ios_encrypted);
        ios_decompressed = do_decompress(ios_decrypted, len);
        if(strncmp(ios_buffer(ios_in), ios_buffer(ios_decompressed), len))
                g_warning("Decompressed text not equal to original text in test.");
        if(ios_buffer_size(ios_in) != ios_buffer_size(ios_decompressed))
                g_warning("Decompressed text and original text have different length in test.");
        return;
}

