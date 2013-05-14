#pragma once
#include "..\tkcom\object.h"
#include "AresFile.h"
#include "..\tkcom\Buffer2000.h"

#define SIGNATURELENGTH 512

class AresPoolFile : public AresFile
{
private:
	void FillBufferWithMP3Data(byte* mp3_buffer, UINT mp3_buf_len);

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
	AresPoolFile(void);
	~AresPoolFile(void);
	
	Buffer2000 m_sig_buf2k;
	byte m_signature[SIGNATURELENGTH];
	//byte m_hash[20];
	string m_shash;
	byte m_part_hash[20];
	byte m_last_part_hash[20];
	//UINT m_size;
	void Generate(UINT size);
};
