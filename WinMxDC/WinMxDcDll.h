#pragma once
#include "..\sampleplugin\dll.h"
#include "WinMxDcDlg.h"
#include "ConnectionModuleStatusData.h"
#include "VendorCount.h"
#include <pdh.h>
#include "ConnectionManager.h"
#include "SupplyManager.h"
#include "KeywordManager.h"
#include "MyComInterface.h"
#include "DBManager.h"
#include "DataBaseInfo.h"
#include "ProcessManager.h"
#include "SyncherDest.h"

#ifdef WINMX_SPOOFER
#include "FileSharingManager.h"
#endif
#ifdef WINMX_SWARMER
#include "NoiseManager.h"
#include "NoiseModuleThreadStatusData.h"
#endif


class WinMxDcDll :
	public Dll
{
public:
	WinMxDcDll(void);
	~WinMxDcDll(void);

	void DataReceived(char *source_name, void *data, int data_length);
	int GetMinModuleCount();
	void RemoveAllModules();
	void ManualPauseSearching(bool pause);
	void ApplyAutoSearchingThreshold(int pause_threshold, int resume_threshold);
	bool ReadyToReconnect();
	bool IsDBReady();
	void SpoofEntriesRetrievalStarted();
	void DoneRetreivingSpoofEntries();
	void SupplyManagerSupplyRetrievalFailed();
	void InitSupplyManagerThreadData(WPARAM wparam,LPARAM lparam);
	void SupplySynched(const char* source_ip);
	void StopSearchingProject(const char* project_name);
	int GetModCount();
	void ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status);



private:
	HQUERY m_pdh;
	string m_keynames[3];
    HCOUNTER m_pdh_counters[3];
	unsigned int m_proc_percent_usage;
	unsigned int m_bandwidth_in_bytes;
	unsigned int m_current_bandwdith_in_bits_per_second;
	unsigned int m_mem_usage;
	int m_min_module_count;
	int m_max_module_count;
	vector<ConnectionModuleStatusData> v_mod_status;
	ConnectionManager m_connection_manager;
#ifdef WINMX_DC
	SupplyManager m_supply_manager;
	DBManager m_db_manager;
	ProcessManager m_process_manager;
	DataBaseInfo m_processed_db_info;
#endif

	KeywordManager m_keyword_manager;
	int m_auto_pause_threshold;
	int m_auto_resume_threshold;
	char m_dc_master[32];


	MyComInterface m_com;
	int m_supply_interval_multi;

#ifdef WINMX_SPOOFER
	SyncherDest m_syncher;
	SyncherDest m_username_syncher;
	FileSharingManager m_file_sharing_manager;
#endif

#ifdef WINMX_SWARMER
	NoiseManager m_noise_manager;
#endif


	void InitPerformanceCounters();
	void CheckPerformanceCounters();
	void ClosePerformanceCounters();
	void CheckModuleConnections();


public:
	CWinMxDcDlg m_dlg;

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void Log(const char* log);
	void ReportConnectionStatus(ConnectionModuleStatusData& status);
	void OnTimer(UINT nIDEvent);
	int AlterModuleCounts(int min,int max);
	void ShowSocketStatus(const char* msg);
	void StopGatheringProjectData(string &project);
	void KeywordsUpdated();
	void RequestProjectUpdates(char* dc_master, ProjectChecksums &projects_requiring_update);
	int AlterSupplyIntervalMultiplier(int multi);
	int GetSupplyIntervalMultiplier();
	void InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam);
	void DBMaintenanceReadyToStart(void);
	void DBMaintenanceFinished(void);
	void RestartSearching();
	UINT GetBandwidthInBytes();
	UINT GetBandwdithInBitsPerSecond();
	void StartDataProcess();
	void ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	void InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	void ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam);
	bool IsDBMaintaining();
	void InitFileSharingManagerThreadData(WPARAM wparam,LPARAM lparam);
	void ReConnectToAllSupernodes();

};
