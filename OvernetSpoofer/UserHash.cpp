#include "StdAfx.h"
#include "userhash.h"
#include "crc.h"

UserHash::UserHash(void)
{
	Clear();
}

//
//
//
UserHash::~UserHash(void)
{
	Clear();
}

//
//
//
bool UserHash::operator < (const UserHash& other)const
{
	if(this->m_checksum < other.m_checksum)
		return true;
	else
		return false;
}

//
//
//
bool UserHash::operator ==(UserHash &other)
{
	return (this->m_checksum == other.m_checksum);
}

//
//
//
void UserHash::Clear()
{
	memset(&m_user_hash,0,sizeof(m_user_hash));
	m_checksum=0;
	hs_peer_hashes.clear();
}

//
//
//
void UserHash::SetHash(byte* hash)
{
	memcpy(m_user_hash,hash,16);
	CryptoPP::CRC32 crc;
	crc.Update(m_user_hash,16);
	crc.CalculateDigest((byte*)&m_checksum,m_user_hash,16);
}