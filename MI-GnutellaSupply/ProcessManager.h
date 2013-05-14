#pragma once
#include "manager.h"
#include "ProcessDBInterface.h"
#include "ProcessManagerThreadData.h"
//#include "ProcessManagerDBMaintenanceThreadData.h"
#include "DataBaseInfo.h"

class CCriticalSection;
class ProcessManager :	public Manager
{
public:
	// Public Member Functions
	ProcessManager(void);
	~ProcessManager(void);
	void InitParent(GnutellaSupplyDll *parent);
	bool IsProcessing();
	bool IsAllDone();
	void StartProcessRawData(DataBaseInfo& db_info);
	void InitDemandThreadData(WPARAM wparam,LPARAM lparam);
	void InitSupplyThreadData(WPARAM wparam,LPARAM lparam);
	//void InitMaintenanceThreadData(WPARAM wparam,LPARAM lparam);
	void MaintenanceFinished();
	//void DemandProcessFinished();
	//void SupplyProcessFinished();
	void KillThread();
	void Reset();
	void GetProcessProgress(int& demand_progress, int& supply_progress/*, int& dns*/);
	bool IsMaintaining();
	void KeywordUpdated(vector<ProjectKeywords>& projects);
	void ProjectIsReadyToBeProcessed(UINT project_id);
	void GetMaintainProgress(int& maintenance_progress){ maintenance_progress=m_maintenance_progress;}


	// Public Data Members
	static ProcessDBInterface g_db_demand_interface;
	static ProcessDBInterface g_db_supply_interface;
	static ProcessDBInterface g_db_hash_interface;
	static ProcessDBInterface g_processed_db_interface;
	int m_maintenance_progress;

	vector<ProjectKeywords> v_projects;

private:
	// Private Data Members
	CCriticalSection* p_demand_critical_section;
	CCriticalSection* p_supply_critical_section;
	
	ProcessManagerThreadData* p_demand_thread_data;
	ProcessManagerThreadData* p_supply_thread_data;

	//CCriticalSection* p_maintenance_critical_section;
	//ProcessManagerDBMaintenanceThreadData* p_maintenance_thread_data;

//	static bool m_is_processing;
	static bool m_is_maintaining;
	static bool m_is_all_done;

	static bool m_is_demand_processing;
	static bool m_is_supply_processing;

	CWinThread* m_demand_thread;
	CWinThread* m_supply_thread;


	DataBaseInfo m_db_info;

public:
	// delete old raw data from data base
	//void DBMaintenance(vector<ProjectKeywords>& keywords);
	void DBMaintenance();

};
