// BlubsterHost.cpp

#include "stdafx.h"
#include "BlubsterHost.h"

//
//
//
BlubsterHost::BlubsterHost()
{
	Clear();
	m_port=41170;
}

//
//
//
BlubsterHost::~BlubsterHost()
{
}

//
//
//
void BlubsterHost::Clear()
{
	m_ip=0;
	m_port=0;
	m_connected=0;
	m_num_connections=0;
	memset(&m_vendor, 0, sizeof(m_vendor));
	m_connected_before = false;
}

//
//
//
bool BlubsterHost::operator<(const BlubsterHost &host)const
{
	if(m_ip<host.m_ip)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool BlubsterHost::operator<(const unsigned int &ip)const
{
	if(m_ip<ip)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool BlubsterHost::operator==(const BlubsterHost &host)const
{
	if(m_ip==host.m_ip)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool BlubsterHost::SortByNumConnections(BlubsterHost &host)
{
	if(m_num_connections<host.m_num_connections)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
void BlubsterHost::SetVendor(const char* vendor)
{
	if(strlen(vendor)+1 <= sizeof(m_vendor))
		strcpy(m_vendor, vendor);
}

//
//
//
void BlubsterHost::ClearVendor()
{
	memset(&m_vendor, 0, sizeof(m_vendor));
}