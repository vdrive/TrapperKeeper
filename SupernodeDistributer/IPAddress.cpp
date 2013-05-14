#include "StdAfx.h"
#include "ipaddress.h"

IPAddress::IPAddress(void)
{
	m_ip = 0;
	m_port = 0;
	Clear();
	m_up_time = CTime::GetCurrentTime();
	m_down_time = CTime::GetCurrentTime();
}

//
//
//
IPAddress::IPAddress(IPAndPort &ip)
{
	m_ip = ip.m_ip;
	m_port = ip.m_port;
	Clear();
	m_up_time = CTime::GetCurrentTime();
	m_down_time = CTime::GetCurrentTime();
}

//
//
//
IPAddress::~IPAddress(void)
{
}

//
//
//
bool IPAddress::operator == (const IPAddress & other)const
{
	if(m_ip == other.m_ip)
		if(m_port == other.m_port)
			return true;
	return false;
}

//
//
//
bool  IPAddress::operator < (const IPAddress& other)const
{
	if(this->m_ip < other.m_ip)
		return true;
	else
		return false;
}

//
//
//
bool IPAddress::operator != (const IPAddress & other)const
{
	if(m_ip == other.m_ip)
		if(m_port == other.m_port)
			return false;
	return true;
}

//
//
//
void IPAddress::Clear()
{
	m_is_up = false;
	//m_up_time = CTime::GetCurrentTime();
	m_connecting = false;
	m_rack_name = "NULL";
}
