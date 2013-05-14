/* 
** FILE: IP.h
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

#pragma once
#include "StdAfx.h"

class IPRange;
class IP
{
// friends
	friend class IPRange;

// functions
public:
	IP(void);
	IP(unsigned long nIP);
	IP(const IP &rData);
	~IP(void);

	inline void SetIP(unsigned long nIP) { m_nIP = ((nIP>>24)&0xff) | (nIP<<24) | ((nIP>>8)&0x0000ff00) | ((nIP<<8)&0x00ff0000); }
	inline unsigned long GetIP(void) const { return ( ((m_nIP>>24)&0xff) | (m_nIP<<24) | ((m_nIP>>8)&0x0000ff00) | ((m_nIP<<8)&0x00ff0000) ); }

	inline unsigned long GetIPForDB(void) const { return m_nIP; }
	inline void SetIPFromDB(unsigned long nIP) { m_nIP = nIP; }

	inline bool SameRange(const IP &rIP) const { return ( (m_nIP&0xffffff00) == (rIP.m_nIP&0xffffff00) ); }

	string ToString(bool nZeroPadded = true) const;

	IP &operator=(const IP &rRight);
	inline bool operator==(const IP &rRight) const { return m_nIP == rRight.m_nIP; }
	inline bool operator!=(const IP &rRight) const { return m_nIP != rRight.m_nIP; }
	inline friend bool operator==(const IP &rLeft, const IP &rRight) { return rLeft.m_nIP == rRight.m_nIP; }
	inline friend bool operator!=(const IP &rLeft, const IP &rRight) { return rLeft.m_nIP != rRight.m_nIP; }
	inline friend bool operator<(const IP &rLeft, const IP &rRight) { return rLeft.m_nIP < rRight.m_nIP; }
	inline friend bool operator>(const IP &rLeft, const IP &rRight) { return rLeft.m_nIP > rRight.m_nIP; }

// variables
private:
	unsigned long m_nIP;
};
