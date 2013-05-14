#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma once

typedef struct
{
	int pos;
	unsigned int wrapcount;
	unsigned int add_to_lookup;
	unsigned int enc_type;
	unsigned char pad[63];
	unsigned char lookup[256];
	unsigned int pad16[20];
	unsigned int seed;
} FSTCipher;



	/* allocate and init cipher */
	FSTCipher *fst_cipher_create();

	/* free cipher */
	void fst_cipher_free (FSTCipher *cipher);

	/* initialize cipher state */
	/* returns FALSE if enc_type is not supported, TRUE otherwise */
	int fst_cipher_init (FSTCipher *cipher, unsigned int seed, unsigned int enc_type);

	/* encrypt / decrypt a byte of data with cipher */
	unsigned char fst_cipher_clock (FSTCipher *cipher);

	/* encrypt / decrypt a block of data with cipher */
	void fst_cipher_crypt (FSTCipher *cipher, unsigned char *data, int len);

	/* returns encrypted or decrypted enc_type */
	unsigned int fst_cipher_mangle_enc_type (unsigned int seed, unsigned int enc_type);


