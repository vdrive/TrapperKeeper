#pragma once
#include "..\sampleplugin\dll.h"
#include "GnutellaProtectorDlg.h"
#include "LogWindowManager.h"
#include "ConnectionModuleStatusData.h"
#include "VendorCount.h"
#include "ConnectionManager.h"
#include "KeywordManager.h"
#include "SupplyManager.h"
#include <pdh.h>
#include "ProjectStatusReportData.h"
#include "DBManager.h"
#include "DCHeader.h"
#include "..\StatusSource\MyComInterface.h"
#include "DataBaseInfo.h"
#include "ProcessManager.h"
#include "SyncherDest.h"

class GnutellaProtectorDll :
	public Dll
{
public:
	GnutellaProtectorDll(void);
	~GnutellaProtectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	void EnableVendorCounts(BOOL enabled);
	void SendSupplyRetrievalProgress(int progress);

	
	//public data members
	CGnutellaProtectorDlg m_dlg;
	LogWindowManager m_log_window_manager;
	SupplyManager m_supply_manager;

	//public data functions
	void ReportConnectionStatus(ConnectionModuleStatusData &status);
	void Log(char *msg);
	void VendorCountsReady(vector<VendorCount> &vendor_counts,int compression_on_counts);
	void RequestProjectUpdates(char* dc_msater, ProjectChecksums &projects_requiring_update);
	void KeywordsUpdated();
	//void UpdateSupplyProjects(vector<SupplyProject> &supply_projects);
	void StopGatheringProjectData(string &project);
	void ShowLogWindow();
	void OnTimer(UINT nIDEvent);
//	void SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam);
	int AlterModuleCounts(int min,int max);
	void InitDBManagerThreadData(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam);
	void InitProcessManagerThreadData(WPARAM wparam,LPARAM lparam);
	void ProcessManagerDone(WPARAM wparam,LPARAM lparam);
	void StartDataProcess();
	void ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	void SpoofEntriesRetrievalStarted();
	void InitSupplyManagerThreadData(WPARAM wparam,LPARAM lparam);
	void UpdateSupplyManagerSupplyProjects(WPARAM wparam,LPARAM lparam);
	void AddHost(CString host);

private:
	//private data members
	ProcessManager m_process_manager;
	ConnectionManager m_connection_manager;
	KeywordManager m_keyword_manager;
	DBManager m_db_manager;
	vector<ProjectKeywords> v_new_project_keywords;
	HQUERY m_pdh;
	string m_keynames[3];
    HCOUNTER m_pdh_counters[3];
	vector<ConnectionModuleStatusData> v_mod_status;
	CTime m_running_since_time;
	unsigned int m_proc_percent_usage;
	unsigned int m_bandwidth_in_bytes;
	unsigned int m_current_bandwdith_in_bits_per_second;
	unsigned int m_mem_usage;
	int m_min_module_count;
	int m_max_module_count;
//	DBInterface m_db;
	MyComInterface m_com;
	DataBaseInfo m_processed_db_info;
	DataBaseInfo m_gnutella_raw_db_info;
	char m_dc_master[32];
	SyncherDest m_syncher;


	//private data functions
	//void ExtractProjectStatusReportData(ConnectionModuleStatusData &status);
	void SendAllPartialProjectStatusReportData();
	void InitPerformanceCounters();
	void CheckPerformanceCounters();
	void ClosePerformanceCounters();
	void SendStatus();
	void CheckModuleConnections();
	void SendSupplyRetrievalStartingTime();
	void SendSupplyRetrievalEndingTime();

public:
	void DBMaintenanceReadyToStart(void);
	void DBMaintenanceFinished(void);
	void RetrieveSupplyEntries();
};
