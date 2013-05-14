// IPAndPort.cpp: implementation of the IPAndPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IPAndPort.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IPAndPort::IPAndPort()
{
	memset(this, 0, sizeof(IPAndPort));
	m_up_since = CTime::GetCurrentTime();
}

//
//
//
IPAndPort::IPAndPort(int ip, int port)
{
	m_ip = ip;
	m_port = port;
	m_up_since = CTime::GetCurrentTime();
}

//
//
//
bool IPAndPort::operator ==(const IPAndPort& right)const
{
	if(this->m_ip == right.m_ip && this->m_port == right.m_port)
		return true;
	return false;
}

//
//
//
bool IPAndPort::operator < (const IPAndPort& right)const
{
	if(this->m_ip < right.m_ip)
		return true;
	else
		return false;
}

//
//
//
bool IPAndPort::IsEqual(const int& ip, const int& port)const
{
	if(this->m_ip == ip && this->m_port == port)
		return true;
	return false;
}
