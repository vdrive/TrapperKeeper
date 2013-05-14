#pragma once
#include "SupernodeHost.h"

class WinMxSocketStatusData
{
public:
	WinMxSocketStatusData(void);
	~WinMxSocketStatusData(void);
	void Clear();

	// Public Data Members
	vector<SupernodeHost> v_other_hosts;
	SupernodeHost m_host;
	CTime m_connect_time;
	CTime m_up_time;
};