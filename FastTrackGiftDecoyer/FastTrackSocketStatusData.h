#pragma once

#include "SupernodeHost.h"

class FastTrackSocketStatusData
{
public:
	// Public Member Functions
	FastTrackSocketStatusData();
	~FastTrackSocketStatusData();
	void Clear();

	// Public Data Members
	SupernodeHost m_host;
	string m_remote_vendor;
	CTime m_connect_time;
	CTime m_up_time;
};