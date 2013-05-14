#include "StdAfx.h"
#include "aresftbanclient.h"

AresFTBanClient::AresFTBanClient(const char* ip,byte *hash)
{
	m_ip=ip;
	memcpy(m_hash,hash,20);
	m_create_time=CTime::GetCurrentTime();
}

AresFTBanClient::~AresFTBanClient(void)
{
}
