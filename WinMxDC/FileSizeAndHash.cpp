#include "StdAfx.h"
#include "filesizeandhash.h"
#include "crc.h"

FileSizeAndHash::FileSizeAndHash(void)
{
	Clear();
}

FileSizeAndHash::~FileSizeAndHash(void)
{
}

//
//
//
bool  FileSizeAndHash::operator < (const FileSizeAndHash& other)const
{
	if(this->m_checksum < other.m_checksum)
		return true;
	else
		return false;
}

//
//
//
bool FileSizeAndHash::operator ==(FileSizeAndHash &other)
{
	return (this->m_checksum == other.m_checksum);
}

//
//
//
void FileSizeAndHash::Clear()
{
	m_checksum=m_filesize=0;
	m_filename.Empty();
}

//
//
//
void FileSizeAndHash::SetHashChecksum()
{
	CryptoPP::CRC32 crc;
	char* filename = new char[m_filename.GetLength()+1];
	strcpy(filename,m_filename);
	crc.Update((const byte*)filename,m_filename.GetLength()+1);
	crc.CalculateDigest((byte*)&m_checksum,(const byte*)filename,m_filename.GetLength()+1);
	delete [] filename;
}

//
//
//
void FileSizeAndHash::SetFilenameAndSize(const char* filename, UINT size)
{
	m_filename = filename;
	m_filesize = size;
	SetHashChecksum();
}
