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
	bool IsProcessing();
	bool IsAllDone();
	void StartProcessRawData(DataBaseInfo& db_info, vector<ProjectKeywords>* keywords);
	void InitSupplyThreadData(WPARAM wparam,LPARAM lparam);
	void MaintenanceFinished();
	void SupplyProcessFinished();
	void KillThread();
	void Reset();
	void GetProcessProgress(int& supply_progress);
	bool IsMaintaining();


	// Public Data Members
	static ProcessDBInterface g_db_supply_interface;
	vector<ProjectKeywords> v_projects;

private:
	// Private Data Members
	CCriticalSection* p_supply_critical_section;
	
	ProcessManagerThreadData* p_supply_thread_data;

	static bool m_is_maintaining;
	static bool m_is_all_done;

	static bool m_is_supply_processing;


public:
	// delete old raw data from data base
	void DBMaintenance(void);
};
