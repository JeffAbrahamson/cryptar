/*  encryption.c
 *  Copyright (C) 2002, 2003, 2004 by
 *  Jeff Abrahamson
 *  Adam O'Donnell
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

/* Credits to Daniel Richards <kyhwana@world-net.co.nz> for the general
 * structure of this code.  I borrowed quite a bit from his encrypt example
 * that was packaged with libtomcrypt for this work.  
 *
 * Even though he has made his code public domain, it is good karma to 
 * recognize the author. -- adam
 */

#include <glib.h>
#include <stdio.h>
#include <mycrypt_custom.h>

#include "encryption.h"
#include "ios.h"
#include "options.h"


typedef struct crypto_instance {
	int cipher_idx;
	int hash_idx;
	int prng_idx;
	int ivsize;
	int ks;   /* cipher_descriptor[].keysize() expects this to be an int.
               hash_memory expects this to be an unsigned long.
               what to do? */
	/* And now that comment as a haiku:
	 *
	 * struct provides an int
	 * but the hash desires a long
	 * before being called 
	 *
	 * */

	/* The real issue is that ks is the cipher key size, and
	 * the hash routine cares not about the lowly cipher, all
	 * it wants to know is how much data to write.  So lets
	 * create a second variable that contains the same value.
	 * This won't kill us. */
	unsigned long hashkeylen;
	char hashkey[MAXBLOCKSIZE]; 
	prng_state prng;
} CryptoInstance;

CryptoInstance *g_crypto = NULL;       /* global crypto state */



/* Create our crypto information structure */
void init_crypto(void)
{
#ifndef CLEARTEXT
	char IV[MAXBLOCKSIZE];
	int x, err;

	g_crypto = (CryptoInstance *)malloc(sizeof(CryptoInstance));

	/* Register the cipher, hasher, and PRNG.  This can be done
	 * ONCE per execution over many blocks.  This is a later
	 * optimization we should consider making */
	if (register_cipher(&aes_desc) == -1) {
		g_error("Unable to register cipher, exiting...");
		exit(1);
	}
	if (register_hash(&sha256_desc) == -1) {
		g_error("Unable to register hash, exiting...");
		exit(1);
	}
	/* This is a default rng, we can switch to another later. */
	if (register_prng(&yarrow_desc) == -1) {
		g_error("Unable to register PRNG, exiting...");
		exit(1);
	}

	/* Grab the indexes of the cipher, hash, and prng */
	g_crypto->cipher_idx = find_cipher("aes");
	if (g_crypto->cipher_idx == -1) {
                g_error("Unable to find defined cipher aes, exiting...");
                exit(1);
        }

	/* Start up SHA256 to hash the input key */
	g_crypto->hash_idx = find_hash("sha256");
	if (g_crypto->hash_idx == -1) {
		g_error("Unable to find defined hash SHA256, exiting...");
		exit(1);
	}

	/* Start up Yarrow PRNG */
	g_crypto->prng_idx = find_prng("yarrow");
	if (g_crypto->prng_idx == -1) {
		g_error("Unable to find defined PRNG Yarrow, exiting...");
		exit(1);
	}

	/* Get the IV size */
	g_crypto->ivsize = cipher_descriptor[g_crypto->cipher_idx].block_length;

	/* Generate a hashed key that is the block length of the cipher. */
	g_crypto->ks = hash_descriptor[g_crypto->hash_idx].hashsize;
	if (cipher_descriptor[g_crypto->cipher_idx].keysize(&(g_crypto->ks)) != CRYPT_OK) {
		g_error("Invalid keysize from the hash function, exiting...");
		exit(1);
	}

	/* Copy this over to hashkeylen */
	g_crypto->hashkeylen = g_crypto->ks;

	/* Set up the prng */
	if ((err = rng_make_prng(128, find_prng("yarrow"), &(g_crypto->prng), NULL)) != CRYPT_OK) {
		g_error("Error setting up PRNG, exiting...");	
		exit(1);
	}

	/* Burn off a few bytes as a test */
	x = yarrow_read(IV, g_crypto->ivsize, &(g_crypto->prng));
	if (x != g_crypto->ivsize) {
		g_error("Error reading PRNG for IV required.\n");
		exit(1);
	}

	/* Memset the key to all 0's */
	memset(g_crypto->hashkey, 0, MAXBLOCKSIZE);	

	/* Lets not be stupid.  Generate a random key.  We can change this
	 * later if we must. */
	generate_random_key();

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("init_crypto() completed successfully.");

#endif /* not CLEARTEXT */
	return;
}



void print_key()
{
	unsigned int i;

        /* Presumably only called if some verbose option is set (i.e.,
           caller wants this), so don't check here for verbosity.
        */
	g_message("Key: ");
	for (i = 0; i < g_crypto->hashkeylen-1; i++) {
		g_message("%x:", g_crypto->hashkey[i]);
	}
	g_message("%x\n", g_crypto->hashkey[i]);
	return;
}



/* Hash the key */
void string_to_key(Key key)
{
	int err;

        g_assert(key);
	if ((err = hash_memory(g_crypto->hash_idx, key, strlen(key), g_crypto->hashkey, &g_crypto->hashkeylen)) != CRYPT_OK) {
		g_error("Error hashing key: %s", error_to_string(err));
		exit(1);
	}

        if(int_option(kOption_verbose) & VERBOSE_FLOW)
                g_message("Key hashed successfully.\n");
	return;
}



void set_key(char *inputkey, int len)
{
	int i;

        g_assert(inputkey);
        g_assert(len > 0);      /* is zero an error or just negative? */
	memset(g_crypto->hashkey, 0, MAXBLOCKSIZE);	
	for (i = 0; i < len && i < MAXBLOCKSIZE; i++) {
		g_crypto->hashkey[i] = inputkey[i];
	}

	return;
}



/* Generate a random key.
 * Simple way: generate 512 random bytes and feed this to
 * string_to_key() */
void generate_random_key()
{
	char random_source_key[RANDOM_KEYSIZE+1];
	int x;

	x = yarrow_read(random_source_key, RANDOM_KEYSIZE, &(g_crypto->prng));
	if (x != RANDOM_KEYSIZE) {
		g_error("Error reading PRNG for IV required.\n");
		exit(1);
	}

	/* Always null terminate strings. */
	random_source_key[RANDOM_KEYSIZE] = '\0';
	string_to_key(random_source_key);
        if(int_option(kOption_verbose) & VERBOSE_FLOW_PLUS)
                print_key();
	return;
}



IOSBuf *encrypt(IOSBuf *ios_in)
{
#ifndef CLEARTEXT
        IOSBuf *ios_out;
	int x, err;
	char IV[MAXBLOCKSIZE];
	char *ciphertext;
	/* Reference for the cipher running in counter mode */
	symmetric_CTR ctr;
	
        g_assert(ios_in);
	ios_out = ios_new();

	/* Generate some random for the IV */
	x = yarrow_read(IV, g_crypto->ivsize, &(g_crypto->prng));
	if (x != g_crypto->ivsize) {
		g_error("Error reading PRNG for IV required.\n");
		exit(1);
	}

	/* Start up the cipher counter mode */
	if ((err = ctr_start(g_crypto->cipher_idx, IV, g_crypto->hashkey, g_crypto->ks, 0, &ctr)) != CRYPT_OK) {
		g_error("ctr_start error: %s\n", error_to_string(err));
		exit(1);
	}

	/* Append the IV to the current IOSBuf */
	ios_append(ios_out, IV, g_crypto->ivsize);
	/* Give us some room to write our ciphertext data */
	ciphertext = (char *)malloc(ios_buffer_size(ios_in));
	if (ios_buffer(ios_in) == NULL) {
		g_warning("Empty IOSBuf in encrypt, returning NULL\n");
		return NULL;
	}
	if ((err = ctr_encrypt(ios_buffer(ios_in), ciphertext, ios_buffer_size(ios_in), &ctr)) != CRYPT_OK) {
		g_error("ctr_encrypt error: %s\n", error_to_string(err));
		exit(1);
	}

	ios_append(ios_out, ciphertext, ios_buffer_size(ios_in));
#ifdef DEBUG_CRYPTO
	g_message("Encrypt completed.\nInput/Output size: %i/%i\n", ios_buffer_size(ios_in), ios_buffer_size(ios_out));
#endif

	/* And so I don't forget... */
	free((void *)ciphertext);

#else
        IOSBuf *ios_out;
        ios_out = ios_new_copy(ios_in);
#endif
        return ios_out;
}



IOSBuf *decrypt(IOSBuf *ios_in)
{
#ifndef CLEARTEXT
        IOSBuf *ios_out;
	int err;
	char IV[MAXBLOCKSIZE];
	char *plaintext;
	/* Reference for the cipher running in counter mode */
	symmetric_CTR ctr;

  g_assert(ios_in);
	ios_out = ios_new();

	/* Grab the IV */
	strncpy(IV, ios_buffer(ios_in), g_crypto->ivsize);
	/* Figure out how much data we are dealing with, minus the IV */
	plaintext = (char *) malloc(ios_buffer_size(ios_in)-g_crypto->ivsize);

	/* Start up the cipher counter mode */
	if ((err = ctr_start(g_crypto->cipher_idx, IV, g_crypto->hashkey, g_crypto->ks, 0, &ctr)) != CRYPT_OK) {
		g_error("ctr_start error: %s\n", error_to_string(err));
		exit(1);
	}

	/* Decrypt our data.  Start this not from the beginning of the
	 * string, but skip the number of bytes occupied by the IV.
	 * Do this as well for the length of the buffer being
	 * decrypted as well. */
	if (ios_buffer(ios_in) == NULL) {
		g_warning("Empty IOSBuf in decrypt, returning NULL\n");
		return NULL;
	}
	if ((err = ctr_decrypt(ios_buffer(ios_in)+g_crypto->ivsize, plaintext, ios_buffer_size(ios_in)-g_crypto->ivsize, &ctr)) != CRYPT_OK) {
		g_error("ctr_decrypt error: %s\n", error_to_string(err));
		exit(1);
	}

	/* Append the data to the out buffer. */
	ios_append(ios_out, plaintext, ios_buffer_size(ios_in)-g_crypto->ivsize);

#ifdef DEBUG_CRYPTO
	g_message("Encrypt completed.\nInput/Output size: %i/%i\n", ios_buffer_size(ios_in), ios_buffer_size(ios_out));
#endif

	/* Memory leaks bad! */
	free((void *)plaintext);

#else
        IOSBuf *ios_out;
        ios_out = ios_new_copy(ios_in);
#endif
        return ios_out;
}



gint block_size(gint block_size)
{
#ifndef CLEARTEXT
        return block_size;      /* But really pass back something
                                   that's ok. Call g_warn() if
                                   changed. */
#else
        return block_size;
#endif
}
