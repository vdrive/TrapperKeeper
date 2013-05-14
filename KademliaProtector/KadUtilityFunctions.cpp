#include "StdAfx.h"
#include "kadutilityfunctions.h"

// basic MD4 functions
#define MD4_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD4_G(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define MD4_H(x, y, z) ((x) ^ (y) ^ (z))

// rotates x left n bits
// 15-April-2003 Sony: use MSVC intrinsic to save a few cycles
#ifdef _MSC_VER
#pragma intrinsic(_rotl)
#define MD4_ROTATE_LEFT(x, n) _rotl((x), (n))
#else
#define MD4_ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#endif

// partial transformations
#define MD4_FF(a, b, c, d, x, s) \
{ \
  (a) += MD4_F((b), (c), (d)) + (x); \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

#define MD4_GG(a, b, c, d, x, s) \
{ \
  (a) += MD4_G((b), (c), (d)) + (x) + (UINT)0x5A827999; \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

#define MD4_HH(a, b, c, d, x, s) \
{ \
  (a) += MD4_H((b), (c), (d)) + (x) + (UINT)0x6ED9EBA1; \
  (a) = MD4_ROTATE_LEFT((a), (s)); \
}

// constants for MD4Transform
#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15

KadUtilityFunctions::KadUtilityFunctions(void)
{
}

KadUtilityFunctions::~KadUtilityFunctions(void)
{
}

void KadUtilityFunctions::HashData(byte* in_string,UINT in_string_len,byte *output)
{
	// time critial
	md4clr(output);
	bool PaddingStarted = false;
	UINT Hash[4];
	Hash[0] = 0x67452301;
	Hash[1] = 0xEFCDAB89;
	Hash[2] = 0x98BADCFE;
	Hash[3] = 0x10325476;
	CFile* data = 0;
	if (in_string)
		data = new CMemFile(in_string,in_string_len);
	UINT Required = in_string_len;
	byte   X[64*128];  
	while (Required >= 64){
        UINT len = Required / 64; 
        if (len > sizeof(X)/(64 * sizeof(X[0]))) 
             len = sizeof(X)/(64 * sizeof(X[0])); 

		data->Read(&X,len*64);

		for (UINT i = 0; i < len; i++) 
        { 
           MD4Transform(Hash, (UINT*)(X + i*64)); 
        }
		Required -= len*64;
	}
	// bytes to read
	Required = in_string_len % 64;
	if (Required != 0){
		data->Read(&X,Required);
	}
	// in byte scale 512 = 64, 448 = 56
	if (Required >= 56){
		X[Required] = 0x80;
		PaddingStarted = TRUE;
		memset(&X[Required + 1], 0, 63 - Required);
		MD4Transform(Hash, (UINT*)X);
		Required = 0;
	}
	if (!PaddingStarted)
		X[Required++] = 0x80;
	memset(&X[Required], 0, 64 - Required);
	// add size (convert to bits)
	UINT in_string_len2 = in_string_len >> 29;
	in_string_len <<= 3;
	memcpy(&X[56], &in_string_len, 4);
	memcpy(&X[60], &in_string_len2, 4);
	MD4Transform(Hash, (UINT*)X);
	md4cpy(output, Hash);
	if(data!=NULL){
		delete data;
	}
	//safe_delete(data);
}

//from emule
void KadUtilityFunctions::MD4Transform(UINT Hash[4], UINT x[16])
{
  UINT a = Hash[0];
  UINT b = Hash[1];
  UINT c = Hash[2];
  UINT d = Hash[3];

  /* Round 1 */
  MD4_FF(a, b, c, d, x[ 0], S11); // 01
  MD4_FF(d, a, b, c, x[ 1], S12); // 02
  MD4_FF(c, d, a, b, x[ 2], S13); // 03
  MD4_FF(b, c, d, a, x[ 3], S14); // 04
  MD4_FF(a, b, c, d, x[ 4], S11); // 05
  MD4_FF(d, a, b, c, x[ 5], S12); // 06
  MD4_FF(c, d, a, b, x[ 6], S13); // 07
  MD4_FF(b, c, d, a, x[ 7], S14); // 08
  MD4_FF(a, b, c, d, x[ 8], S11); // 09
  MD4_FF(d, a, b, c, x[ 9], S12); // 10
  MD4_FF(c, d, a, b, x[10], S13); // 11
  MD4_FF(b, c, d, a, x[11], S14); // 12
  MD4_FF(a, b, c, d, x[12], S11); // 13
  MD4_FF(d, a, b, c, x[13], S12); // 14
  MD4_FF(c, d, a, b, x[14], S13); // 15
  MD4_FF(b, c, d, a, x[15], S14); // 16

  /* Round 2 */
  MD4_GG(a, b, c, d, x[ 0], S21); // 17
  MD4_GG(d, a, b, c, x[ 4], S22); // 18
  MD4_GG(c, d, a, b, x[ 8], S23); // 19
  MD4_GG(b, c, d, a, x[12], S24); // 20
  MD4_GG(a, b, c, d, x[ 1], S21); // 21
  MD4_GG(d, a, b, c, x[ 5], S22); // 22
  MD4_GG(c, d, a, b, x[ 9], S23); // 23
  MD4_GG(b, c, d, a, x[13], S24); // 24
  MD4_GG(a, b, c, d, x[ 2], S21); // 25
  MD4_GG(d, a, b, c, x[ 6], S22); // 26
  MD4_GG(c, d, a, b, x[10], S23); // 27
  MD4_GG(b, c, d, a, x[14], S24); // 28
  MD4_GG(a, b, c, d, x[ 3], S21); // 29
  MD4_GG(d, a, b, c, x[ 7], S22); // 30
  MD4_GG(c, d, a, b, x[11], S23); // 31
  MD4_GG(b, c, d, a, x[15], S24); // 32

  /* Round 3 */
  MD4_HH(a, b, c, d, x[ 0], S31); // 33
  MD4_HH(d, a, b, c, x[ 8], S32); // 34
  MD4_HH(c, d, a, b, x[ 4], S33); // 35
  MD4_HH(b, c, d, a, x[12], S34); // 36
  MD4_HH(a, b, c, d, x[ 2], S31); // 37
  MD4_HH(d, a, b, c, x[10], S32); // 38
  MD4_HH(c, d, a, b, x[ 6], S33); // 39
  MD4_HH(b, c, d, a, x[14], S34); // 40
  MD4_HH(a, b, c, d, x[ 1], S31); // 41
  MD4_HH(d, a, b, c, x[ 9], S32); // 42
  MD4_HH(c, d, a, b, x[ 5], S33); // 43
  MD4_HH(b, c, d, a, x[13], S34); // 44
  MD4_HH(a, b, c, d, x[ 3], S31); // 45
  MD4_HH(d, a, b, c, x[11], S32); // 46
  MD4_HH(c, d, a, b, x[ 7], S33); // 47
  MD4_HH(b, c, d, a, x[15], S34); // 48

  Hash[0] += a;
  Hash[1] += b;
  Hash[2] += c;
  Hash[3] += d;
}

CString KadUtilityFunctions::EncodeBase16(const unsigned char* buffer, unsigned int bufLen)
{
	CString base16_buff;
	static byte base16_alphabet[] = "0123456789ABCDEF";

	for(unsigned int i = 0; i < bufLen; i++) {
		base16_buff += base16_alphabet[buffer[i] >> 4];
		base16_buff += base16_alphabet[buffer[i] & 0xf];
	}

	return base16_buff;
}

byte KadUtilityFunctions::ConvertCharToInt(char ch)
{
	switch(ch){
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A': return 10;
		case 'a': return 10;
		case 'B': return 11;
		case 'b': return 11;
		case 'C': return 12;
		case 'c': return 12;
		case 'D': return 13;
		case 'd': return 13;
		case 'E': return 14;
		case 'e': return 14;
		case 'F': return 15;
		case 'f': return 15;
	}
	return 0;
}
