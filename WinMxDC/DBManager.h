#pragma once
#include "manager.h"
#include "DBInterface.h"
#include "QueryHit.h"
#include "DBManagerThreadData.h"

class WinMxDcDll;
class CCriticalSection;
class DBManager : public Manager
{
public:
	// Public Member Functions
	DBManager(void);
	~DBManager(void);
	void InitParent(WinMxDcDll *parent);
	void InitSupplyThreadData(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteSupplyDataToDatabase(WPARAM wparam,LPARAM lparam);
	void PushQueryHitData(vector<QueryHit> &query_hits);
	void ReportDBStatus(UINT& query_hit_size);
	void TerminateThread();
	void OnOneMinuteTimer();
	bool ReadyToReconnect();

	// Public Data Members
	static DBInterface g_supply_db_interface;

private:
	// Private Data Members
	CCriticalSection *p_supply_critical_section;

	DBManagerThreadData m_supply_thread_data;

	vector<QueryHit *> v_project_query_hits;
	
	bool m_ready_to_write_to_supply_db;
	bool m_db_is_maintaining;
	CTime m_last_db_written_at;

	CFile m_raw_supply_file;
	CWinThread* m_supply_thread;

	// Private Member Functions
	void WriteDataToSupplyDatabase();
public:
	void DBMaintenanceReadyToStart(void);
	void DBMaintenanceFinished(void);
private:
	void WriteAllCacheRawDataToDisk(void);
};
