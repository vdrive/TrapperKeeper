#include "StdAfx.h"
#include ".\searchset.h"
#include "crc.h"

SearchSet::SearchSet(void)
{
	Clear();
}

SearchSet::~SearchSet(void)
{
	hs_peer_hashes.clear();
	hs_meta_data.clear();
	hs_meta_data.clear();
}

bool SearchSet::operator < (const SearchSet& other)const
{
	if(this->m_checksum < other.m_checksum)
		return true;
	else
		return false;
}

bool SearchSet::operator ==(SearchSet &other)
{
	return (this->m_checksum == other.m_checksum);
}

void SearchSet::Clear()
{
	//memset(&m_keyword_hash, 0, sizeof(m_keyword_hash));
	//memset(&m_file_hash, 0, sizeof(m_file_hash));
	memset(&m_hash,0,sizeof(m_hash));
	hs_peer_hashes.clear();
	m_is_file_hash=false;
	//v_server_IPs.clear();
	hs_meta_data.clear();
}

void SearchSet::SetHash(byte* hash)
{
	memcpy(m_hash,hash,16);
	CryptoPP::CRC32 crc;
	crc.Update(m_hash,16);
	crc.CalculateDigest((byte*)&m_checksum,m_hash,16);
}

/*
void SearchSet::InsertServerIP(const char* ip)
{
	bool found=false;
	for(UINT i=0;i<v_server_IPs.size();i++)
	{
		if(v_server_IPs[i].Compare(ip)==0)
		{
			found=true;
			break;
		}
	}
	if(!found)
		v_server_IPs.push_back(ip);
}
*/

bool SearchSet::IsHashCloseEnough(const byte hash)
{
	if(hash == m_hash[0])
		return true;
	else
		return false;
}

/*
bool SearchSet::IsFileHash(const byte* right)
{
	if(memcmp(right,m_file_hash,16)==0)
		return true;
	else
		return false;
}

bool SearchSet::IsKeywordHash(const byte* right)
{
	if(memcmp(right,m_keyword_hash,16)==0)
		return true;
	else
		return false;
}

bool SearchSet::IsKeywordHashCloseEnough(const byte hash)
{
	if(m_keyword_hash_set)
	{
		if(hash == m_keyword_hash[0])
			return true;
		else
			return false;
	}
	return false;
}

void SearchSet::SetFileHash(byte* hash,const char* server_ip)
{
	m_server_ip=server_ip;
	memcpy(m_file_hash,hash,16);
	m_file_hash_set=true;
	m_keyword_hash_set=false;
	CryptoPP::CRC32 crc;
	crc.Update(m_file_hash,16);
	crc.CalculateDigest((byte*)&m_checksum,m_file_hash,16);
}

void SearchSet::SetFileHash(byte* hash)
{
	memcpy(m_file_hash,hash,16);
	m_file_hash_set=true;
	m_keyword_hash_set=false;
	CryptoPP::CRC32 crc;
	crc.Update(m_file_hash,16);
	crc.CalculateDigest((byte*)&m_checksum,m_file_hash,16);
}

void SearchSet::SetKeywordHash(byte* hash)
{
	memcpy(m_keyword_hash,hash,16);
	m_keyword_hash_set=true;
	m_file_hash_set=false;
	CryptoPP::CRC32 crc;
	crc.Update(m_keyword_hash,16);
	crc.CalculateDigest((byte*)&m_checksum,m_keyword_hash,16);
}
*/
