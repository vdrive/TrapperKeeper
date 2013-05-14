#ifndef __giFTHASH_H__
#define __giFTHASH_H__

//#include "giFT.h"

/**
 * Update the 4-byte small hash.  After all of the calls to this
 * routine, the return value should be written as a 4-byte little-endian
 * integer, and appended to the 16-byte MD5 hash of the first 307200
 * bytes of the file, to obtain the 20-byte hash.
 */
unsigned int hashSmallHash(byte *data, size_t len, unsigned int hash);

/**
 * This routine procudes the 2-byte checksum that ends up as part of the
 * URL to the file.  Pass in the 20-byte concatenation of the MD5 hash
 * of the first 307200 bytes of the file, and the 4-byte output of the
 * hashSmallHash routine (little-endian).
 */
unsigned short hashChecksum(byte hash[20]);

#endif
