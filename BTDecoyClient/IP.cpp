/* 
** FILE: IP.cpp
**
** ABSTRACT:
**   Object to hold an IP (dword)
**
** AUTHOR:
**   Dylan Douglas
**
** CREATION DATE:
**   06/20/2005
**
** NOTES:
**   None
*/

#include "IP.h"

//
//
//
IP::IP(void)
: m_nIP(0)
{
}

//
//
//
IP::IP(unsigned long nIP)
{
	m_nIP = ((nIP>>24)&0xff) | (nIP<<24) | ((nIP>>8)&0x0000ff00) | ((nIP<<8)&0x00ff0000);
}

//
//
//
IP::IP(const IP &rData)
: m_nIP(rData.m_nIP)
{
}

//
//
//
IP::~IP(void)
{
}

//
//
//
string IP::ToString(bool nZeroPadded /*= true*/) const
{
	char aBuf[23];

	if( nZeroPadded )
	{
		sprintf( aBuf, "%03d.%03d.%03d.%03d", ((m_nIP & 0xff000000)>>24), ((m_nIP & 0x00ff0000)>>16),
			((m_nIP & 0x0000ff00)>>8), (m_nIP & 0x000000ff) );
	}
	else
	{
		sprintf( aBuf, "%d.%d.%d.%d", ((m_nIP & 0xff000000)>>24), ((m_nIP & 0x00ff0000)>>16),
			((m_nIP & 0x0000ff00)>>8), (m_nIP & 0x000000ff) );
	}

	return string( aBuf );
}

//
//
//
IP &IP::operator=(const IP &rRight)
{
	m_nIP = rRight.m_nIP;

	return (*this);
}

