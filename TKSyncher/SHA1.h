//AUTHOR:  Dominik Reichl 
//MODIFIED BY:  Ty Heath (Base32Encode() and commenting)
//LAST MODIFIED:  3.14.2003

#ifndef ___SHA1_H___
#define ___SHA1_H___

#include <stdio.h> // Needed for file access
#include <memory.h> // Needed for memset and memcpy
#include <string.h> // Needed for strcat and strcpy

#define LITTLE_ENDIAN
#define MAX_FILE_READ_BUFFER 8000

class SHA1
{
public:
	// Rotate x bits to the left
	#define ROL32(value, bits) (((value)<<(bits))|((value)>>(32-(bits))))

	#ifdef LITTLE_ENDIAN
		#define SHABLK0(i) (block->l[i] = (ROL32(block->l[i],24) & 0xFF00FF00) \
			| (ROL32(block->l[i],8) & 0x00FF00FF))
	#else
		#define SHABLK0(i) (block->l[i])
	#endif

	#define SHABLK(i) (block->l[i&15] = ROL32(block->l[(i+13)&15] ^ block->l[(i+8)&15] \
		^ block->l[(i+2)&15] ^ block->l[i&15],1))

	// SHA1 rounds
	#define R0(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK0(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
	#define R1(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
	#define R2(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0x6ED9EBA1+ROL32(v,5); w=ROL32(w,30); }
	#define R3(v,w,x,y,z,i) { z+=(((w|x)&y)|(w&x))+SHABLK(i)+0x8F1BBCDC+ROL32(v,5); w=ROL32(w,30); }
	#define R4(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0xCA62C1D6+ROL32(v,5); w=ROL32(w,30); }

	typedef union {
		unsigned char c[64];
		unsigned long l[16];
	} SHA1_WORKSPACE_BLOCK;

	// Two different formats for ReportHash()
	enum { REPORT_HEX = 0, REPORT_DIGIT = 1 };

	// Constructor and Destructor
	SHA1();
	virtual ~SHA1();

	unsigned long m_state[5];
	unsigned long m_count[2];
	unsigned char m_buffer[64];
	unsigned char m_digest[20];

	void Reset();

	// Update the hash value with an array or an entire file
	void Update(unsigned char* data, unsigned int len);
	bool HashFile(char *szFileName);
	bool HashPartOfFile(char *szFileName,UINT start,UINT count);  //Ty Heath, 6.13.2003

	// Finalize hash (call when you are done)
	void Final();

	void ReportHash(char *szReport, unsigned char uReportType = REPORT_HEX);

	//call to get the 20 byte raw hash.  (call only after calling Final())
	void GetHash(unsigned char *uDest);

	//call to create a base 32 encoding of a hash and store in a 33 byte char array (1 byte for the null char at the end).
	void Base32Encode(char *result);

private:
	char base32Alphabet[32];
	// Private SHA-1 transformation
	void Transform(unsigned long state[5], unsigned char buffer[64]);
};

#endif // ___SHA1_H___
