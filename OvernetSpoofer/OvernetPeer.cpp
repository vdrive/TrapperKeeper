#include "StdAfx.h"
#include ".\overnetpeer.h"
#include "crc.h"

OvernetPeer::OvernetPeer(void)
{
	Clear();
}

OvernetPeer::~OvernetPeer(void)
{
}

void OvernetPeer::Clear()
{
	memset(this,0,sizeof(OvernetPeer));
}


bool OvernetPeer::operator < (const OvernetPeer& other)const
{
	if(this->m_checksum < other.m_checksum)
		return true;
	else
		return false;
}

bool OvernetPeer::operator ==(OvernetPeer &other)
{
	return (this->m_checksum == other.m_checksum);
}

void OvernetPeer::SetHashChecksum()
{
	CryptoPP::CRC32 crc;
	crc.Update(m_id,16);
	crc.CalculateDigest((byte*)&m_checksum,m_id,16);
}

//
//
//
void OvernetPeer::SetID(byte* id, UINT ip, short port)
{
	memcpy(m_id, id, 16);
	m_ip = ip;
	m_port =port;
	SetHashChecksum();
}

bool OvernetPeer::IsHashCloseEnough(const byte hash)
{
	if(hash == m_id[0])
		return true;
	else
		return false;
}