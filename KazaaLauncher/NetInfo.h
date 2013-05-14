#pragma once

class NetInfo
{
public:
	NetInfo(void);
	~NetInfo(void);

	DWORD m_process_id;
	DWORD m_port;
};
