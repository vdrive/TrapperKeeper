#pragma once
#include "KazaaProcess.h"

class GetRealProcessIDThreadData
{
public:
	GetRealProcessIDThreadData(void);
	~GetRealProcessIDThreadData(void);
	
	HWND m_hwnd;
	vector<KazaaProcess> v_kazaa_processes;
	DWORD m_process_id;
	bool m_ending_thread;
};
