// GnutellaSocketStatusData.h
#pragma once

#include "GnutellaHost.h"

class GnutellaSocketStatusData
{
public:
	// Public Member Functions
	GnutellaSocketStatusData();
	~GnutellaSocketStatusData();
	void Clear();

	// Public Data Members
	GnutellaHost m_host;
	string m_remote_vendor;
	CTime m_connect_time;
	CTime m_up_time;
};