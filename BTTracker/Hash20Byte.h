#pragma once

class Hash20Byte
{
// functions
public:
	Hash20Byte(void);
	Hash20Byte(const BYTE aHash[20]);
	Hash20Byte(const CHAR aHash[20]);
	Hash20Byte(BYTE aHash[20]);
	Hash20Byte(char aHash[20]);
	Hash20Byte(const Hash20Byte &rHash);
	~Hash20Byte(void) {}

	Hash20Byte &operator=(const Hash20Byte &rRight);
	Hash20Byte &operator=(const BYTE aRight[20]);
	Hash20Byte &operator=(const CHAR aRight[20]);
	bool operator==(const Hash20Byte &rRight) const;
	bool operator!=(const Hash20Byte &rRight) const;
	friend bool operator<(const Hash20Byte &rLeft, const Hash20Byte &rRight);
	friend bool operator>(const Hash20Byte &rLeft, const Hash20Byte &rRight);

	void FromHexString(const char *pData);
	void FromEscapedHexString(const char *pData);

	inline const BYTE *ToBytePtr(void) const { return m_aHash; };
	const char *CopyPrintableToCharBuf(char *pBuf) const;
	void ZeroHash(void);
	inline bool IsZero(void) const { return ( m_aHash[0] == 0 && m_aHash[1] == 0 && m_aHash[2] == 0 &&
		m_aHash[3] == 0 && m_aHash[4] == 0 && m_aHash[5] == 0 && m_aHash[6] == 0 && m_aHash[7] == 0 &&
		m_aHash[8] == 0 && m_aHash[9] == 0 && m_aHash[10] == 0 && m_aHash[11] == 0 && m_aHash[12] == 0 &&
		m_aHash[13] == 0 && m_aHash[14] == 0 && m_aHash[15] == 0 && m_aHash[16] == 0 && m_aHash[17] == 0 &&
		m_aHash[18] == 0 && m_aHash[19] == 0 ); }
	inline bool IsMajorityZero(void) const { return ( ( m_aHash[0] | m_aHash[1] | m_aHash[2] | m_aHash[3] ) == 0 ||
		( m_aHash[1] | m_aHash[2] | m_aHash[3] | m_aHash[4] ) == 0 ||
		( m_aHash[2] | m_aHash[3] | m_aHash[4] | m_aHash[5] ) == 0 ||
		( m_aHash[3] | m_aHash[4] | m_aHash[5] | m_aHash[6] ) == 0 ||
		( m_aHash[4] | m_aHash[5] | m_aHash[6] | m_aHash[7] ) == 0 ||
		( m_aHash[5] | m_aHash[6] | m_aHash[7] | m_aHash[8] ) == 0 ||
		( m_aHash[6] | m_aHash[7] | m_aHash[8] | m_aHash[9] ) == 0 ||
		( m_aHash[7] | m_aHash[8] | m_aHash[9] | m_aHash[10] ) == 0 ||
		( m_aHash[8] | m_aHash[9] | m_aHash[10] | m_aHash[11] ) == 0 ||
		( m_aHash[9] | m_aHash[10] | m_aHash[11] | m_aHash[12] ) == 0 ||
		( m_aHash[10] | m_aHash[11] | m_aHash[12] | m_aHash[13] ) == 0 ||
		( m_aHash[11] | m_aHash[12] | m_aHash[13] | m_aHash[14] ) == 0 ||
		( m_aHash[12] | m_aHash[13] | m_aHash[14] | m_aHash[15] ) == 0 ||
		( m_aHash[13] | m_aHash[14] | m_aHash[15] | m_aHash[16] ) == 0 ||
		( m_aHash[14] | m_aHash[15] | m_aHash[16] | m_aHash[17] ) == 0 ||
		( m_aHash[15] | m_aHash[16] | m_aHash[17] | m_aHash[18] ) == 0 ||
		( m_aHash[16] | m_aHash[17] | m_aHash[18] | m_aHash[19] ) == 0 ); }

// variables
private:
	BYTE m_aHash[20];
};
