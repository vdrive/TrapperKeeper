#pragma once

class FTHash
{
public:
	FTHash(void);
	~FTHash(void);

	char * HashEncode64();
	char* HashEncode16();
	BOOL HashDecode16(const char *fthash);
	UINT GetFileSize();

	byte m_data[36];
private:
	char * HexEncode (const unsigned char *data, int src_len);
	unsigned char * HexDecode (const char *data, int *dst_len);
	char * Base64Encode (const unsigned char *data, int src_len);


};
