#include "StdAfx.h"
#include "supernodecollector.h"

SupernodeCollector::SupernodeCollector(void)
{
	memset(&m_ip, 0, sizeof(m_ip));
	m_last_init_response_time = CTime::GetCurrentTime() - CTimeSpan(0,2,0,0);
}

//
//
//
SupernodeCollector::~SupernodeCollector(void)
{
}

//
//
//
bool SupernodeCollector::operator == (const SupernodeCollector& sc) const
{
	if(strcmp(this->m_ip, sc.m_ip)==0)
		return true;
	else
		return false;
}

//
//
//
bool SupernodeCollector::operator == (char* ip) const
{
	if(strcmp(this->m_ip, ip)==0)
		return true;
	else
		return false;
}
