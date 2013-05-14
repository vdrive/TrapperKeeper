#pragma once
#include "..\sampleplugin\dll.h"
#include "MemoryScanner.h"
#include "WinMxLauncherDlg.h"

class WinMxLauncherDll :
	public Dll
{
public:

	enum config_data_type
	{
		Winmx_Path,
		Num_Instances,
		Launching_Interval,
		Unknown
	};

	WinMxLauncherDll(void);
	~WinMxLauncherDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void OnTimer(UINT nIDEvent);
	 //overriding the base Dll function to receive data from my other dlls
	bool ReceivedDllData(AppID from_app_id, void* input_data, void* output_data);

private:
	void ReadInConfig(void);
	void WriteConfig(void);
	WinMxLauncherDll::config_data_type GetConfigEntryType(char * entry_string);
	void LaunchWinmx(void);
	int GetNumberOfWinmxRunning(void);
	void CheckWinMxNum();
	void KillWinMx(int num_to_kill);
	void KillAllWinMx();

	bool m_run_winmx;
	MemoryScanner m_memory_scanner;
	int m_max_num_winmx;
	CString m_winmx_path;
	int m_launching_interval;
	CWinMxLauncherDlg m_dlg;
};
