#pragma once
#include <string>
using namespace std;

class Hash20Byte
{
// functions
public:
	Hash20Byte(void);
	Hash20Byte(const unsigned char aHash[20]);
	Hash20Byte(const char aHash[20]);
	Hash20Byte(unsigned char aHash[20]);
	Hash20Byte(char aHash[20]);
	Hash20Byte(const Hash20Byte &rHash);
	~Hash20Byte(void) {}

	Hash20Byte &operator=(const Hash20Byte &rRight);
	Hash20Byte &operator=(const unsigned char aRight[20]);
	Hash20Byte &operator=(const char aRight[20]);
	bool operator==(const Hash20Byte &rRight) const;
	bool operator!=(const Hash20Byte &rRight) const;
	friend bool operator<(const Hash20Byte &rLeft, const Hash20Byte &rRight);
	friend bool operator>(const Hash20Byte &rLeft, const Hash20Byte &rRight);

	void FromHexString(const char *pData);
	void FromEscapedHexString(const char *pData);

	inline const unsigned char *ToBytePtr(void) const { return m_aHash; };
	const char *ToPrintableCharPtr(char *pBuf) const;
	void ZeroHash(void);
	bool IsZero(void) const;

// variables
private:
	unsigned char m_aHash[20];
};
