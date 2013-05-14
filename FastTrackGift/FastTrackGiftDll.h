#pragma once
#include "..\sampleplugin\dll.h"
#include "FastTrackGiftDlg.h"
#include "ConnectionModuleStatusData.h"
#include "VendorCount.h"
#include <pdh.h>
#include "ConnectionManager.h"
#include "SupplyManager.h"
#include "KeywordManager.h"
#include "MyComInterface.h"
#include "DBManager.h"
#include "NoiseManager.h"
#include "NoiseModuleThreadStatusData.h"
#include "FileSharingManager.h"
#include "DataBaseInfo.h"
#include "ProcessManager.h"
#include "KazaaControllerDestSyncher.h"

class FastTrackGiftDll :
	public Dll
{
public:
	FastTrackGiftDll(void);
	~FastTrackGiftDll(void);
	void DataReceived(char *source_name, void *data, int data_length);
	int GetMinModuleCount();
	void RemoveAllModules();
	void ManualPauseSearching(bool pause);
	void ApplyAutoSearchingThreshold(int pause_threshold, int resume_threshold);
	void FileSharingManagerSupplyRetrievalFailed();
	void SendSupplyRetrievalProgress(int progress);
	void EnableVendorCounts(BOOL enable);
	bool ReadyToReconnect();

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
	SupplyManager m_supply_manager;
	KeywordManager m_keyword_manager;
	DBManager m_db_manager;
	KazaaControllerDestSyncher m_syncher;
	int m_auto_pause_threshold;
	int m_auto_resume_threshold;
	char m_dc_master[32];


#ifdef FILE_SERVER_ENABLED
	NoiseManager m_noise_manager;
#endif
#ifdef SHARING_ENABLED
	FileSharingManager m_file_sharing_manager;
#endif
	MyComInterface m_com;
	int m_supply_interval_multi;
	DataBaseInfo m_processed_db_info;
#ifdef DATA_COLLECTION_ENABLED
#ifndef NO_PROCESSING
	ProcessManager m_process_manager;
#endif
#endif
	void InitPerformanceCounters();
	void CheckPerformanceCounters();
	void ClosePerformanceCounters();
	void CheckModuleConnections();
	void SendSupplyRetrievalStartingTime();
	void SendSupplyRetrievalEndingTime();



public:
	CFastTrackGiftDlg m_dlg;

	void ConnectToSupernode(const char* ip, int port);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void Log(const char* log);
//	void Search(const char* search_string);
	void ReportConnectionStatus(ConnectionModuleStatusData& status);
	void VendorCountsReady(vector<VendorCount> &vendor_counts);
	void OnTimer(UINT nIDEvent);
	int AlterModuleCounts(int min,int max);
	void ShowSocketStatus(const char* msg);
	
	//TODO: IsDBMaintaining()
	//TODO: StopGatheringProjectData()
	void StopGatheringProjectData(string &project);
	void FastTrackGiftDll::KeywordsUpdated();
	void RequestProjectUpdates(char* dc_master, ProjectChecksums &projects_requiring_update);
	int AlterSupplyIntervalMultiplier(int multi);
	int GetSupplyIntervalMultiplier();
	void InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam);
	void DBMaintenanceReadyToStart(void);
	void DBMaintenanceFinished(void);
	void RestartSearching();
	int GetModCount();
	UINT GetBandwidthInBytes();
	UINT GetBandwdithInBitsPerSecond();
	void ReportNoiseManagerStatus(vector<NoiseModuleThreadStatusData> &status);
	void StartDataProcess();
	void ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	void InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	void ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam);
	bool IsDBMaintaining();
	void SpoofEntriesRetrievalStarted();
	void InitFileSharingManagerThreadData(WPARAM wparam,LPARAM lparam);
	void DoneRetreivingSpoofEntries();
	void UpdateFileSharingManagerSupplyProjects(WPARAM wparam,LPARAM lparam);
	void ReConnectToAllSupernodes();
};
