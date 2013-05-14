#pragma once
#include "IPAddress.h"

class SupernodeCollector
{
public:
	SupernodeCollector(void);
	~SupernodeCollector(void);
	bool operator == (const SupernodeCollector& sc) const;
	bool operator == (char* ip) const;

	CTime m_last_init_response_time;
	vector<IPAddress> v_supernodes;
	char m_ip[16];
};
