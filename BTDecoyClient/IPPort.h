/* 
** FILE: IPPort.h
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

#pragma once
#include <string>
using namespace std;

//
//
//
class IPPort
{
// functions
public:
	IPPort(void);
	IPPort(unsigned long nIP, unsigned short nPort);
	IPPort(const IPPort &rData);
	~IPPort(void);

	inline void SetIP(unsigned long nIP) { m_nIP = ((nIP>>24)&0xff) | (nIP<<24) | ((nIP>>8)&0x0000ff00) | ((nIP<<8)&0x00ff0000); }
	inline void SetPort(unsigned short nPort) { m_nPort = (((nPort>>8) & 0xff) | (nPort<<8)); }
	inline unsigned long GetIP(void) const { return ( ((m_nIP>>24)&0xff) | (m_nIP<<24) | ((m_nIP>>8)&0x0000ff00) | ((m_nIP<<8)&0x00ff0000) ); }
	inline unsigned short GetPort(void) const { return ((m_nPort>>8) & 0xff) | (m_nPort<<8); }

	string ToString(bool nZeroPadded = true) const;

	IPPort &operator=(const IPPort &rRight);
	inline bool operator==(const IPPort &rRight) const { return ( (m_nIP == rRight.m_nIP) && (m_nPort == rRight.m_nPort) ); }
	inline bool operator!=(const IPPort &rRight) const { return ( (m_nIP != rRight.m_nIP) || (m_nPort != rRight.m_nPort) ); }
	friend inline bool operator<(const IPPort &rLeft, const IPPort &rRight) { return ( (rLeft.m_nIP < rRight.m_nIP) || (rLeft.m_nIP == rRight.m_nIP && rLeft.m_nPort < rRight.m_nPort) ); }
	friend inline bool operator>(const IPPort &rLeft, const IPPort &rRight) { return ( (rLeft.m_nIP > rRight.m_nIP) || (rLeft.m_nIP == rRight.m_nIP && rLeft.m_nPort > rRight.m_nPort) ); }

// variables
private:
	unsigned long m_nIP;
	unsigned short m_nPort;
};
