// IPAndPort.h: interface for the IPAndPort class.
//
//////////////////////////////////////////////////////////////////////
#ifndef IP_AND_PORT_H
#define IP_AND_PORT_H

class IPAndPort  
{
public:
	IPAndPort();
	IPAndPort(int ip, int port);
	bool IsEqual(const int& ip, const int& port)const;
	bool operator ==(const IPAndPort& right)const;
	bool operator < (const IPAndPort&  right)const;

	int m_ip;
	int m_port;
	CTime m_up_since;
};

#endif
