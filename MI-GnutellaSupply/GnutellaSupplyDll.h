#pragma once
#include "..\sampleplugin\dll.h"
#include "GnutellaSupplyDlg.h"
#include "LogWindowManager.h"
//#include "LogfileManager.h"
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

class GnutellaSupplyDll :
	public Dll
{
public:
	GnutellaSupplyDll(void);
	~GnutellaSupplyDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	void EnableVendorCounts(BOOL enabled);
	
	//public data members
	CGnutellaSupplyDlg m_dlg;
	LogWindowManager m_log_window_manager;
	SupplyManager m_supply_manager;

	//public data functions
	void ReportConnectionStatus(ConnectionModuleStatusData &status);
	void Log(char *msg);
	void VendorCountsReady(vector<VendorCount> &vendor_counts);
	void RequestProjectUpdates(char* dc_msater, ProjectChecksums &projects_requiring_update);
	void KeywordsUpdated();
	//void UpdateSupplyProjects(vector<SupplyProject> &supply_projects);
	void StopGatheringProjectData(string &project);
	void ShowLogWindow();
	void OnTimer(UINT nIDEvent);
//	void SupplyManagerThreadDataReady(WPARAM wparam,LPARAM lparam);
//	void InitLogfileManagerThreadData(WPARAM wparam,LPARAM lparam);
	int AlterModuleCounts(int min,int max);
//	void ReadyToWriteLogfileDataToFile(WPARAM wparam,LPARAM lparam);
	int AlterSupplyIntervalMultiplier(int multi);
	int GetSupplyIntervalMultiplier();
	void InitDBManagerDemandThreadData(WPARAM wparam,LPARAM lparam);
	void InitDBManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteDataToDemandDatabase(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteDataToSupplyDatabase(WPARAM wparam,LPARAM lparam);
	void InitProcessManagerDemandThreadData(WPARAM wparam,LPARAM lparam);
	void ProcessManagerDemandDone(WPARAM wparam,LPARAM lparam);
	void StartDataProcess();
	//void InitDBManagerMaintenanceThreadData(WPARAM wparam,LPARAM lparam);
	void ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	void InitProcessManagerSupplyThreadData(WPARAM wparam,LPARAM lparam);
	void ProcessManagerSupplyDone(WPARAM wparam,LPARAM lparam);
	void InitProcessManagerReverseDNSThreadData(WPARAM wparam,LPARAM lparam);
	void ProcessManagerReverseDNSDone(WPARAM wparam,LPARAM lparam);
	bool IsDBMaintaining();
	bool ReadyToSearch(); //is ready to do supply collection?
	void ProjectIsReadyToBeProcessed(UINT project_id);

private:
	//private data members
	ProcessManager m_process_manager;
	ConnectionManager m_connection_manager;
	KeywordManager m_keyword_manager;
//	LogfileManager m_logfile_manager;
	DBManager m_db_manager;
	vector<ProjectKeywords> v_new_project_keywords;
	HQUERY m_pdh;
	string m_keynames[3];
    HCOUNTER m_pdh_counters[3];
	vector<ConnectionModuleStatusData> v_mod_status;
//	vector<ProjectStatusReportData> v_project_status_report_data;
	CTime m_running_since_time;
	unsigned int m_proc_percent_usage;
	unsigned int m_bandwidth_in_bytes;
	unsigned int m_current_bandwdith_in_bits_per_second;
	unsigned int m_mem_usage;
	int m_min_module_count;
	int m_max_module_count;
	int m_supply_interval_multi;
	DBInterface m_db;
	MyComInterface m_com;
	DataBaseInfo m_processed_db_info;

	//private data functions
	//void ExtractProjectStatusReportData(ConnectionModuleStatusData &status);
	void InitPerformanceCounters();
	void CheckPerformanceCounters();
	void ClosePerformanceCounters();
	void SendStatus();
	void CheckModuleConnections();

public:
	void DBMaintenanceReadyToStart(void);
	void DBMaintenanceFinished(void);
	void StartDBMaintenance();

};
