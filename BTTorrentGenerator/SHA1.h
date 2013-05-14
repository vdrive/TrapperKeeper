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
	typedef union {
		unsigned char c[64];
		unsigned int l[16];
	} SHA1_WORKSPACE_BLOCK;

	// Two different formats for ReportHash()
	enum { REPORT_HEX = 0, REPORT_DIGIT = 1 };

	// Constructor and Destructor
	SHA1();
	virtual ~SHA1();

	unsigned int m_state[5];
	unsigned int m_count[2];
	unsigned char m_buffer[64];
	unsigned char m_digest[20];

	void Reset();

	// Update the hash value with an array or an entire file
	void Update(unsigned char* data, unsigned int len);
	bool HashFile(char *szFileName);
	bool HashFile(char *szFileName,int sleep,volatile int &b_kill);
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
	void Transform(unsigned int state[5], unsigned char buffer[64]);
};

#endif // ___SHA1_H___
