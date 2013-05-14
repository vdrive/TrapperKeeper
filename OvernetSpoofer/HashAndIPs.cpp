#include "StdAfx.h"
#include ".\hashandips.h"
#include "crc.h"

HashAndIPs::HashAndIPs(void)
{
	Clear();
}

HashAndIPs::~HashAndIPs(void)
{
}

bool HashAndIPs::operator < (const HashAndIPs& other)const
{
	if(this->m_checksum < other.m_checksum)
		return true;
	else
		return false;
}

bool HashAndIPs::operator ==(HashAndIPs &other)
{
	return (this->m_checksum == other.m_checksum);
}

void HashAndIPs::Clear()
{
	memset(&m_hash,0,sizeof(m_hash));
	memset(&m_spoof_hash,0,sizeof(m_spoof_hash));
	m_filesize=0;
	v_server_IPs.clear();
	v_server_spoofed_IPs.clear();
}

void HashAndIPs::SetHash(byte* hash)
{
	memcpy(m_hash,hash,16);
	for(int j=0; j<16; j++)
	{
		if(j < 16-1)
			m_spoof_hash[j] ^= m_hash[j+1];
	}
	CryptoPP::CRC32 crc;
	crc.Update(m_hash,16);
	crc.CalculateDigest((byte*)&m_checksum,m_hash,16);
}

void HashAndIPs::InsertServerIP(IPAndUserHash& ip)
{
	bool found=false;
	for(UINT i=0;i<v_server_IPs.size();i++)
	{
		if(v_server_IPs[i] == ip)
		{
			found=true;
			break;
		}
	}
	if(!found)
		v_server_IPs.push_back(ip);
}

void HashAndIPs::InsertServerSpoofedIP(IPAndUserHash& ip)
{
	bool found=false;
	for(UINT i=0;i<v_server_spoofed_IPs.size();i++)
	{
		if(v_server_spoofed_IPs[i] == ip)
		{
			found=true;
			break;
		}
	}
	if(!found)
		v_server_spoofed_IPs.push_back(ip);
}