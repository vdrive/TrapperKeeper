#pragma once

class AntiLeakFile
{
	CString EncodeBase16(const unsigned char* buffer, unsigned int bufLen)
	{
		CString base16_buff;
		static byte base16_alphabet[] = "0123456789ABCDEF";

		for(unsigned int i = 0; i < bufLen; i++) {
			base16_buff += base16_alphabet[buffer[i] >> 4];
			base16_buff += base16_alphabet[buffer[i] & 0xf];
		}

		return base16_buff;
	}

	string m_file_name;
	string m_str_hash;
	byte m_hash[20];
	UINT m_size;
	
public:
	AntiLeakFile(const char* file_name,const char* str_hash,byte* hash,UINT size);
	~AntiLeakFile(void);
};
