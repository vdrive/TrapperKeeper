
#include "stdafx.h"
#include "SupernodeHost.h"

SupernodeHost::SupernodeHost(void)
{
	m_ip = m_port = 0;
}

SupernodeHost::~SupernodeHost(void)
{
}

//
//
//
bool  SupernodeHost::operator < (const SupernodeHost& other)const
{
	if(this->m_ip < other.m_ip)
		return true;
	else
		return false;
}

//
//
//
bool SupernodeHost::operator ==(SupernodeHost &host)
{
	return (m_ip == host.m_ip) && (m_port == host.m_port);
}

//
//
//
void SupernodeHost::Clear()
{
	memset(this,0,sizeof(SupernodeHost));
}

//
//
//
void SupernodeHost::GetIP(char* ip)
{
	sprintf(ip,"%u.%u.%u.%u",(m_ip>>0)&0xFF,(m_ip>>8)&0xFF,(m_ip>>16)&0xFF,(m_ip>>24)&0xFF);
}

//
//
//
void SupernodeHost::SetIP(const char* ip)
{
	if(strlen(ip)!=0)
	{	
		int ip1,ip2,ip3,ip4;
		sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
		m_ip = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
	}
}

void SupernodeHost::setport(short port)
{
	m_port = port;
}

short SupernodeHost::getport()
{
	return m_port;
}