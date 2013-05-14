#pragma once

class SystemInfoData
{
public:
	SystemInfoData(void);
	~SystemInfoData(void);
	
	UINT m_total_physical_memory;
	float m_total_harddisk_space;
	char m_cpu_info[128];
	OSVERSIONINFOEX m_os_info;
	BOOL b_OsVersionInfoEx;
	char m_mac_address[32];
	TCHAR m_computer_name[MAX_COMPUTERNAME_LENGTH + 1];
};
