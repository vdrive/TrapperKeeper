#pragma once

class CCriticalSection;
class RegistryMonitorThreadData
{
public:
	RegistryMonitorThreadData(void);
	~RegistryMonitorThreadData(void);

	enum vals
	{
		NumberOfEvents=2
	};
	HANDLE m_events[NumberOfEvents];
	CString m_database_dir;

	CCriticalSection* p_critical_section;
};
