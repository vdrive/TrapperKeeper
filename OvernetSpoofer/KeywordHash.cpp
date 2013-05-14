#include "StdAfx.h"
#include "keywordhash.h"
#include "crc.h"


KeywordHash::KeywordHash(void)
{
	Clear();
}

KeywordHash::~KeywordHash(void)
{
	Clear();
}

//
//
//
bool KeywordHash::operator < (const KeywordHash& other)const
{
	if(this->m_checksum < other.m_checksum)
		return true;
	else
		return false;
}

//
//
//
bool KeywordHash::operator ==(KeywordHash &other)
{
	return (this->m_checksum == other.m_checksum);
}

//
//
//
void KeywordHash::Clear()
{
	memset(&m_hash,0,sizeof(m_hash));
	memset(&m_my_user_hash,0,sizeof(m_my_user_hash));
	m_checksum=0;
	m_keyword.Empty();
	vp_project_keywords.clear();
}

//
//
//
void KeywordHash::SetHash(byte* hash)
{
	memcpy(m_hash,hash,16);
	CryptoPP::CRC32 crc;
	crc.Update(m_hash,16);
	crc.CalculateDigest((byte*)&m_checksum,m_hash,16);
}

//
//
//
void KeywordHash::SetUserHash(byte* hash)
{
	memcpy(m_my_user_hash,hash,16);
}
