// RecordHeader.cpp: implementation of the Entry class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RecordHeader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RecordHeader::RecordHeader()
{

	Ksig = 0x6C33336C;
	unknown = 0;
	isShared = 1;

	memset(filename,0,sizeof(filename));
	memset(filedirectory,0,sizeof(filedirectory));


}

RecordHeader::~RecordHeader()
{

}

void RecordHeader::WriteHeader(CFile * dbfile)
{
	dbfile->Write(&Ksig,sizeof(int));
	dbfile->Write(&recordlength,sizeof(int));
	dbfile->Write(filename,(UINT)strlen(filename)+1);
	dbfile->Write(filedirectory,(UINT)strlen(filedirectory)+1);
	dbfile->Write(&filesize,sizeof(int));
	dbfile->Write(&filedate,sizeof(int));
	dbfile->Write(&lastshared,sizeof(int));
	dbfile->Write(&unknown,sizeof(int));
	dbfile->Write(&isShared,sizeof(bool));
	dbfile->Write(&recordcount,sizeof(int));
}