#pragma once
#include "..\tkcom\object.h"

class Swarm : public Object
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

public:
	inline int CompareTo(Object *object){
		Swarm *host=(Swarm*)object;
		return stricmp(m_shash.c_str(),host->m_shash.c_str());
	}
	Swarm(const char* file_name, int file_size,byte *hash,byte* hash_set);
	~Swarm(void);

	string m_file_name;
	int m_file_size;
	byte m_hash[16];
	string m_shash;
	byte *m_hash_set;
	//string m_hash;
	//string m_hash_set;
};
