#pragma once
#include "manager.h"
#include "ProcessDBInterface.h"
#include "ProcessManagerThreadData.h"
#include "DataBaseInfo.h"

class CCriticalSection;
class ProcessManager :	public Manager
{
public:
	// Public Member Functions
	ProcessManager(void);
	~ProcessManager(void);
	void InitParent(PioletDCDll *parent);
	bool IsProcessing();
	bool IsAllDone();
	void StartProcessRawData(DataBaseInfo& db_info, vector<ProjectKeywords>& keywords);
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void MaintenanceFinished();
	void ProcessFinished();
	void KillThread();
	void Reset();
	void GetProcessProgress(int& demand_progress, int& supply_progress);


	// Public Data Members
	static ProcessDBInterface g_db_interface;

private:
	// Private Data Members
	CCriticalSection* p_critical_section;
	ProcessManagerThreadData* p_thread_data;

	static bool m_is_processing;
	static bool m_is_maintaining;
	static bool m_is_all_done;
	vector<ProjectKeywords> v_projects;

public:
	// delete old raw data from data base
	void DBMaintenance(void);
};
