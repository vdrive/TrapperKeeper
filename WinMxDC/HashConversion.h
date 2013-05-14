#pragma once
#define HASH_LEN 16
class HashConversion
{
public:
	HashConversion(void);
	~HashConversion(void);
	char * HashEncode64();
	char* HashEncode16();
	BOOL HashDecode16(const char *fthash);
	UINT GetFileSize();

	byte m_data[32+1];
private:
	char * HexEncode (const unsigned char *data, int src_len);
	unsigned char * HexDecode (const char *data, int *dst_len);
	char * Base64Encode (const unsigned char *data, int src_len);
};
