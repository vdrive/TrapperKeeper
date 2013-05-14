#pragma once

#include "SupernodeHost.h"

class SLSKSocketStatusData
{
public:
	// Public Member Functions
	SLSKSocketStatusData();
	~SLSKSocketStatusData();
	void Clear();

	// Public Data Members
	SupernodeHost m_host;
	//string m_remote_vendor;
	CTime m_connect_time;
	CTime m_up_time;
};