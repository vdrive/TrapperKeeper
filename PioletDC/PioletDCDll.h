#pragma once
#include "..\sampleplugin\dll.h"
#include "..\TKCom\TKComInterface.h"
#include "PioletDCDlg.h"
#include "ConnectionManager.h"
#include "ProjectChecksums.h"
#include "KeywordManager.h"
#include "DatabaseInfo.h"
#include "..\StatusSource\MyComInterface.h"
#include "DBManager.h"
#include "SupplyManager.h"
#include <pdh.h>
#include "ProcessManager.h"


class VendorCount;
class PioletDCDll :	public Dll
{
public:
	PioletDCDll(void);
	~PioletDCDll(void);
	
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	void OnTimer(UINT nIDEvent);

	void RequestProjectUpdates(char* dc_msater, ProjectChecksums &projects_requiring_update);
	void KeywordsUpdated();
	void DemandSupplyDataReady(ConnectionModuleStatusData& status);
	void InitDBManagerThreadData(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam);
	void ReportVendorCounts(vector<VendorCount>* vendor_counts);
	void InitProcessManagerThreadData(WPARAM wparam,LPARAM lparam);
	void ProcessManagerDone(WPARAM wparam,LPARAM lparam);
	void StartDataProcess();
	void ProcessManagerMaintenanceDone(WPARAM wparam,LPARAM lparam);
	void DBMaintenanceReadyToStart(void);
	void DBMaintenanceFinished(void);


	CPioletDCDlg m_dlg;

private:
	void InitPerformanceCounters();
	void CheckPerformanceCounters();
	void ClosePerformanceCounters();

	HQUERY m_pdh;
	string m_keynames[3];
    HCOUNTER m_pdh_counters[3];
	unsigned int m_proc_percent_usage;
	unsigned int m_bandwidth_in_bytes;
	unsigned int m_current_bandwdith_in_bits_per_second;
	unsigned int m_mem_usage;

	MyComInterface m_com;
	ConnectionManager* p_connection_manager;
	KeywordManager* p_keyword_manager;
	DBManager* p_db_manager;
	SupplyManager* p_supply_manager;
	ProcessManager* p_process_manager;
	DataBaseInfo m_processed_db_info;

public:
	void UpdateHostLimits(UINT max_host, UINT max_host_cache);
};
