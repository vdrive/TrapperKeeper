#pragma once
#include "manager.h"
#include "DBInterface.h"
//#include "QueryHit.h"
#include "Query.h"
#include "DBManagerThreadData.h"

class PioletSpooferDll;
class CCriticalSection;
class DBManager : public Manager
{
public:
	// Public Member Functions
	DBManager(void);
	~DBManager(void);
	void InitParent(PioletSpooferDll *parent);

	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void ReadyToWriteDataToDatabase(WPARAM wparam,LPARAM lparam);
	void PushSpoofData(vector<Query> &queries);
	void ReportDBStatus(UINT& query_size);

	// Public Data Members
	static DBInterface g_db_interface;

private:
	// Private Data Members
	CCriticalSection *p_critical_section;
	DBManagerThreadData m_thread_data;

//	vector<QueryHit *> v_project_query_hits;
	vector<Query *> v_project_queries;
	
	bool m_ready_to_write_to_db;
	bool m_db_is_maintaining;

	// Private Member Functions
	void WriteDataToDatabase();
public:
	void DBMaintenanceReadyToStart(void);
	void DBMaintenanceFinished(void);
private:
	void WriteAllCacheRawDataToDisk(void);
};
