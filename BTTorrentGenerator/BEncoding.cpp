#include "StdAfx.h"
#include ".\bencoding.h"

BEncoding::BEncoding(void)
{
	m_btype=btype::end;
}

BEncoding::~BEncoding(void)
{
}

//
//
//
BEncoding::btype BEncoding::CheckNextByte(CFile *file)
{
	char c;
	file->Read(&c,1);

	switch(c)
	{
		case 'i':
		{
			return btype::integer;
		}
		case 'l':
		{
			return btype::list;
		}
		case 'd':
		{
			return btype::dictionary;
		}
		case 'e':
		{
			return btype::end;
		}
		default:
		{
			file->Seek(-1,CFile::current);	// put the integer character back
			return btype::string;
		}
	}
}

//
//
//
void BEncoding::ReadSelf(CFile *file){}