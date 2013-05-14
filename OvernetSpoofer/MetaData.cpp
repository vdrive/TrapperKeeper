#include "StdAfx.h"
#include ".\metadata.h"
#include "crc.h"

MetaData::MetaData(void)
{
}

MetaData::~MetaData(void)
{
}

bool MetaData::operator < (const MetaData& other)const
{
	if(this->m_checksum < other.m_checksum)
		return true;
	else
		return false;
}

bool MetaData::operator ==(MetaData &other)
{
	return (this->m_checksum == other.m_checksum);
}
//
//
//
void MetaData::Clear()
{
	memset(&m_file_hash, 0, sizeof(m_file_hash));
	memset(&m_keyword_hash, 0, sizeof(m_keyword_hash));

	m_filename.Empty();
	m_filesize=m_checksum=0;
	m_file_type.Empty();
	m_file_format.Empty();
	m_artist.Empty();
	m_album.Empty();
	m_title.Empty();
	m_length.Empty();
	m_bitrate=0;
	m_availability=0;
	m_keyword.Empty();
	m_codec.Empty();
}

void MetaData::SetFileHash(byte* hash)
{
	memcpy(m_file_hash, hash, 16);
	CryptoPP::CRC32 crc;
	crc.Update(m_file_hash,16);
	crc.CalculateDigest((byte*)&m_checksum,m_file_hash,16);
}

void MetaData::SetKeywordHash(byte* hash)
{
	memcpy(m_keyword_hash, hash, 16);
}