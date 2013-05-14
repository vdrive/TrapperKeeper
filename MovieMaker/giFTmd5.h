#ifndef __giFTMD5_H__
#define __giFTMD5_H__

//#include "giFT.h"

typedef unsigned int word32;

struct MD5Context {
    word32 hash[4];
    word32 bytes[2];
    word32 input[16];
};
void byteSwap(word32 *buf, unsigned words);
void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, byte const *buf, unsigned len);
void MD5Final(byte digest[16], struct MD5Context *context);
void MD5Transform(word32 hash[4], word32 const input[16]);

#endif
