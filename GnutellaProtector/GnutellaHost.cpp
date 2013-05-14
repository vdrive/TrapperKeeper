#include "StdAfx.h"
#include "gnutellahost.h"

//
//
//
GnutellaHost::GnutellaHost()
{
	Clear();
}

//
//
//
GnutellaHost::~GnutellaHost()
{
}

//
//
//
void GnutellaHost::Clear()
{
//	memset(this,0,sizeof(GnutellaHost));

	m_host.erase();
	m_ip=0;
	m_port=0;
}

//
//
//
bool GnutellaHost::operator ==(GnutellaHost &host)
{
	if((m_ip==0) || (host.m_ip==0))
	{
		if(strcmp(m_host.c_str(),host.m_host.c_str())!=0)
		{
			return false;
		}
	}
	else
	{
		if(m_ip!=host.m_ip)
		{
			return false;
		}
	}

	if(m_port!=host.m_port)
	{
		return false;
	}

	return true;
}

//
//
//
unsigned int GnutellaHost::IP()
{
	return m_ip;
}

//
//
//
string GnutellaHost::Host()
{
	return m_host;
}

//
//
//
unsigned int GnutellaHost::Port()
{
	return m_port;
}

//
//
//
void GnutellaHost::Host(const char *host)
{
	m_host=host;

	unsigned int ip1,ip2,ip3,ip4;
	if(sscanf(host,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4)==4)
	{
		m_ip=(ip1<<0) | (ip2<<8) | (ip3<<16) | (ip4<<24);
	}
	else
	{
		m_ip=0;
	}
}

//
//
//
void GnutellaHost::Port(unsigned int port)
{
	m_port=port;
}

//
//
//
void GnutellaHost::IP(unsigned int ip)
{
	m_ip=ip;

	char host[15+1];
	sprintf(host,"%u.%u.%u.%u",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF);
	m_host=host;
}