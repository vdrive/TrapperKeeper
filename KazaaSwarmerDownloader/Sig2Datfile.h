#pragma once
#include "sig2datsourcelist.h"
#include "sig2datmetatag.h"

class Sig2DatFile
{
public:
	Sig2DatFile(void);
	~Sig2DatFile(void);

	bool SetFilename(char * filename);
	bool SetHash(char * hash);
	bool SetLength(int length);
	//DWORD CalculateChecksum(unsigned char* lpBuffer, DWORD dwBufferStart, DWORD dwBufferSize);
	int CalculateSize();
	string WriteFile();

private:

	DWORD m_datState;
	DWORD m_datSourceCount;

	// Check the size of this one in hex
	Sig2DatSourceList m_datSourceList;
	DWORD m_metaTagCount;
	Sig2DatMetaTag m_datMetaTag;
	// Tag next
	DWORD m_datStartTime;
	DWORD m_datUnknown1_1;
	DWORD m_datUnknown1_2;
	char m_datLocalFilePath[256+1];
	DWORD m_datUnknownTime;
	DWORD m_datUnknown2;
	DWORD m_datCompletedChunks;
	// chunk range unknown size atm
	DWORD m_datFullRange1;
	DWORD m_datFullRange2;
	BYTE m_datRangeShift;
	DWORD m_datRangeState1;
	DWORD m_datRangeState2;
	DWORD m_datAppendixSignature;
	WORD m_datAppendixSize;
	DWORD m_datAppendixChecksum;

};
