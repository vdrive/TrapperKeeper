#pragma once
#include "manager.h"
#include "DBInterface.h"
//#include "QueryHit.h"
#include "Query.h"
#include "DBManagerThreadData.h"

class GnutellaProtectorDll;
class CCriticalSection;
class DBManager : public Manager
{
public:
	// Public Member Functions
	DBManager(void);
	~DBManager(void);
	void InitParent(GnutellaProtectorDll *parent);

	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam);
	void PushQueryData(vector<Query> &queries);
	void ReportDBStatus(UINT& query_size);
	void TerminateThread();

	// Public Data Members
	static DBInterface g_db_interface;

private:
	// Private Data Members
	CCriticalSection *p_critical_section;
	DBManagerThreadData m_thread_data;

	vector<Query *> v_project_queries;
	
	bool m_ready_to_write_to_db;
	bool m_db_is_maintaining;
	CWinThread* m_thread;

	// Private Member Functions
	void WriteDataToDatabase();
public:
	void DBMaintenanceReadyToStart(void);
	void DBMaintenanceFinished(void);
private:
	void WriteAllCacheRawDataToDisk(void);
};
