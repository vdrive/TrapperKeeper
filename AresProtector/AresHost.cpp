#include "StdAfx.h"
#include "areshost.h"
#include ".\areshost.h"

AresHost::AresHost(unsigned int ip,unsigned short port)
{
	m_fail_count=0;
	m_ip=ip;
	m_port=port;
	char buf[64];
	//char buf2[64];
	sprintf( buf , "%u.%u.%u.%u" , (m_ip>>0)&0xff , (m_ip>>8)&0xff , (m_ip>>16) & 0xff ,(m_ip>>24) & 0xff );  //little endian
	m_sip=buf;

	//TRACE("AresHost::AresHost(%u,%u) host %s:%u created\n",m_ip,port,m_sip.c_str(),port);
}

AresHost::AresHost(const char* ip,unsigned short port)
{
	m_fail_count=0;
	m_sip=ip;
	m_port=port;
	unsigned int ip1,ip2,ip3,ip4;
	sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	m_ip=(ip1<<0)|(ip2<<8)|(ip3<<16)|(ip4<<24);  //little endian

	//m_ip=(ip1<<24)|(ip2<<16)|(ip3<<8)|(ip4<<0);

	//TRACE("AresHost::AresHost(%u,%u) host %s:%u created.\n",m_ip,port,m_sip.c_str(),port);
}


AresHost::~AresHost(void)
{
}

void AresHost::WriteToFile(HANDLE file)
{
	DWORD tmp;
	WriteFile(file,&m_ip,4,&tmp,NULL);
	WriteFile(file,&m_port,2,&tmp,NULL);
}
