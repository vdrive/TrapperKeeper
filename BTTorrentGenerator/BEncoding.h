#pragma once

class BEncoding
{
public:
	BEncoding(void);
	~BEncoding(void);

	enum btype
	{
		string, integer, list, dictionary, end
	};

	btype CheckNextByte(CFile *file);

	virtual void ReadSelf(CFile *file);

	virtual void WriteEncodedData(CFile *pOutputFile) {};

	btype m_btype;
};
