#pragma once

class FTConnectionModule;
class FTConnectionStatus
{
public:
	FTConnectionStatus(void);
	~FTConnectionStatus(void);
	FTConnectionModule* m_mod;
	int m_num_idles;
};
