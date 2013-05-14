#pragma once
#include "manager.h"
#include "DBInterface.h"
#include "QueryHit.h"
#include "Query.h"
#include "DBManagerThreadData.h"

class GnutellaSupplyDll;
class CCriticalSection;
class DBManager : public Manager
{
public:
	// Public Member Functions
	DBManager(void);
	~DBManager(void);
	void InitParent(GnutellaSupplyDll *parent);

	void InitDemandThreadData(WPARAM wparam,LPARAM lparam);
	void InitSupplyThreadData(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteDemandDataToDatabase(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteSupplyDataToDatabase(WPARAM wparam,LPARAM lparam);
	void PushQueryHitData(vector<QueryHit> &query_hits);
	void PushQueryData(vector<Query> &queries);
	void InsertGUID(GUID& guid, CString project, CTime timestamp);
	void ReportDBStatus(UINT& query_size, UINT& query_hit_size);
	void TerminateThread();

	// Public Data Members
	static DBInterface g_demand_db_interface;
	static DBInterface g_supply_db_interface;

private:
	// Private Data Members
	CCriticalSection *p_demand_critical_section;
	CCriticalSection *p_supply_critical_section;

	DBManagerThreadData m_demand_thread_data;
	DBManagerThreadData m_supply_thread_data;

	vector<QueryHit *> v_project_query_hits;
	vector<Query *> v_project_queries;
	
	bool m_ready_to_write_to_demand_db;
	bool m_ready_to_write_to_supply_db;
	bool m_db_is_maintaining;
	CFile m_raw_demand_file;
	CFile m_raw_supply_file;
	CWinThread* m_demand_thread;
	CWinThread* m_supply_thread;


	// Private Member Functions
	void WriteDataToDemandDatabase();
	void WriteDataToSupplyDatabase();
public:
	void DBMaintenanceReadyToStart(void);
	void DBMaintenanceFinished(void);
private:
	void WriteAllCacheRawDataToDisk(void);
};
