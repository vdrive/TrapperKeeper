// RecordHeader.h: interface for the Entry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(RecordHeader_H)
#define RecordHeader_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class RecordHeader  
{
public:
	RecordHeader();
	~RecordHeader();
	void WriteHeader(CFile * file);


	int Ksig;
	int recordlength;
	char  filename[256];
	char  filedirectory[256];
	int filesize;
	int filedate;
	int lastshared;
	int unknown;
	bool isShared;
	int recordcount;
	
};

#endif // RecordHeader
