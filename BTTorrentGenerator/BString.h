#pragma once
#include "BEncoding.h"
#include <string>

class BString :
	public BEncoding
{
public:
	BString(void);
	BString(std::string str);
	virtual ~BString(void);

	void SetString(std::string str);
//	BString &operator=(const BString &str);
	void WriteEncodedData(CFile *pOutputFile);

	// Reading
	unsigned int Len();
	unsigned char *Buf();

	// Writing
	//void ReadSelf(CFile *file);

private:
	unsigned int m_len;
	unsigned char *p_buf;
};
