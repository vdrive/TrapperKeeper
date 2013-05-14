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
	m_filesize=0;
	m_checksum=0;
	m_filename.Empty();
	memset(m_hash,0,sizeof(m_hash));

}

//
//
//
void FileSizeAndHash::SetHashChecksum()
{
	/*
	unsigned short sum = 0;
	int i;

	unsigned char* hash_ptr = m_hash;
	// calculate 2 byte checksum used in the URL from 20 byte fthash
	for (i = 0; i < FST_FTHASH_LEN; i++)
	{
		sum = checksumtable[(*hash_ptr)^(sum >> 8)] ^ (sum << 8);
		hash_ptr++;
	}

	m_checksum = (sum & 0x3fff);
	*/
	CryptoPP::CRC32 crc;
	crc.Update(m_hash,32);
	crc.CalculateDigest((byte*)&m_checksum,m_hash,32);
}

//
//
//
void FileSizeAndHash::SetHashAndSize(const char* hash, UINT size)
{
	memcpy(m_hash, hash, 32);
	m_filesize = size;
	SetHashChecksum();
}
