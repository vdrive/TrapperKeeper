/*
 * $Id: md5.h,v 1.2 2005/03/10 23:08:48 ivan Exp $
 *
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */

#ifndef __MD5_H
#define __MD5_H

/*****************************************************************************/

#define MD5_HASH_LEN 16

typedef struct
{
	unsigned int     buf[4];
	unsigned int     bits[2];
	unsigned char in[64];
} MD5Context;

void MD5Init(MD5Context *context);
void MD5Update(MD5Context *context, unsigned char const *buf, unsigned len);
void MD5Final(unsigned char digest[MD5_HASH_LEN], MD5Context *context);

/*****************************************************************************/

#endif /* __MD5_H */

