#pragma once
#include "..\sampleplugin\dll.h"
#include "StatusSourceDlg.h"
#include "StatusData.h"
#include <pdh.h>
#include "MyComInterface.h"
#include "SystemInfoData.h"
#include "EnumWindowData.h"
#include "FileInfo.h"


class StatusSourceDll :	public Dll
{
public:
	StatusSourceDll(void);
	~StatusSourceDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	bool InitPerformanceCounters();
	bool CheckPerformanceCounters(StatusData &status);
	void ClosePerformanceCounters();

	void ReceivedStatusRequest(char* source, UINT version);

	void FindKaZaAMemUsages(vector<unsigned int> *kazaa_mem_usages);

	void SharedFileCount();
	void OnTimer(UINT nIDEvent);
	void RestartComputer(void);
	void OnKazaaHwnd(WPARAM wparam,LPARAM lparam);

private:
	CStatusSourceDlg m_dlg;

	HQUERY m_pdh;
	string m_keynames[5];
    HCOUNTER m_pdh_counters[5];
	bool m_performance_counters_initialized;
	unsigned int m_shared_file_count; //C:\Fasttrack Shared
	UINT m_movies_shared;			  //C:\Fasttrack Shared\Movies
	UINT m_music_shared;			  //C:\Fasttrack Shared\Music
	UINT m_swarmer_shared;			  //C:\Fasttrack Shared\Swarmer
	CString m_mac_address;

	MyComInterface m_com; //TKCom

	CStdioFile m_log_file;

private:
	void ExploreFolder(char *folder, vector<FileInfo> &files);
	void ReceivedRequestSharedFiles(char* source_name);
	void GetSystemInformation(SystemInfoData& sys_info);
	void GetCPUInfo(CString& cpu_info);
	void SendSystemInfoData(char* dest, SystemInfoData& sys_info);
	void ReceivedNumKazaaUploadsRequest(char* from);
};
