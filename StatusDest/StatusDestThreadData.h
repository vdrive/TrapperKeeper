#pragma once
#include "SourceHost.h"
#include "NetworkBandwidth.h"
#include "..\StatusSource\StatusData.h"

class StatusDestThreadData
{
public:
	StatusDestThreadData(void);
	~StatusDestThreadData(void);
/*
	enum vals
	{
		NumberOfEvents=3
	};
*/
	vector<SourceHost> *pv_hosts;
	vector<NetworkBandwidth*> *pv_network_bandwidths;
//	HANDLE m_events[NumberOfEvents];
	CString m_from;
	StatusData m_status;
	//vector<UINT> v_kazaa_mems;
	HWND m_hwnd;
};
