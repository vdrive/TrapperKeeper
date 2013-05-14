#pragma once
#include "IPAndPort.h"

class IPAddress
{
public:
	void Clear();
	IPAddress(void);
	IPAddress(IPAndPort& ip);
	~IPAddress(void);
	bool operator == (const IPAddress & other)const;
	bool operator < (const IPAddress&  other)const;
	bool operator != (const IPAddress& other)const;

	int m_ip;
	int m_port;
	bool m_connecting;
	bool m_is_up;
	CTime m_up_time;
	CTime m_down_time;
	string m_rack_name;
};
