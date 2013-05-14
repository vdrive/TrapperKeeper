/* 
** FILE: IPPort.cpp
**
** ABSTRACT:
**   Object to hold an IP (dword) and port (ushort) combination
**
** AUTHOR:
**   Dylan Douglas
**
** CREATION DATE:
**   10/18/2004
**
** NOTES:
**   None
*/

#include "IPPort.h"

//
//
//
IPPort::IPPort(void)
: m_nIP(0), m_nPort(0)
{
}

//
//
//
IPPort::IPPort(unsigned long nIP, unsigned short nPort)
{
	m_nIP = ((nIP>>24)&0xff) | (nIP<<24) | ((nIP>>8)&0x0000ff00) | ((nIP<<8)&0x00ff0000);
	m_nPort = nPort;
}

//
//
//
IPPort::IPPort(const IPPort &rData)
: m_nIP(rData.m_nIP), m_nPort(rData.m_nPort)
{
}

//
//
//
IPPort::~IPPort(void)
{
}

//
//
//
string IPPort::ToString(bool nZeroPadded /*= true*/) const
{
	char aBuf[23];

	if( nZeroPadded )
	{
		sprintf( aBuf, "%03d.%03d.%03d.%03d:%05u", ((m_nIP & 0xff000000)>>24), ((m_nIP & 0x00ff0000)>>16),
			((m_nIP & 0x0000ff00)>>8), (m_nIP & 0x000000ff), m_nPort );
	}
	else
	{
		sprintf( aBuf, "%d.%d.%d.%d:%u", ((m_nIP & 0xff000000)>>24), ((m_nIP & 0x00ff0000)>>16),
			((m_nIP & 0x0000ff00)>>8), (m_nIP & 0x000000ff), m_nPort );
	}

	return string( aBuf );
}

//
//
//
IPPort &IPPort::operator=(const IPPort &rRight)
{
	m_nIP = rRight.m_nIP;
	m_nPort = rRight.m_nPort;

	return (*this);
}

