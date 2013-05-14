// DBInterface.h
#pragma once

#include "mysql.h"
#include "Query.h"
#include "QueryHit.h"

class DBInterface  
{
public:
	DBInterface();
	virtual ~DBInterface();

	bool OpenSupplyConnection(CString ip, CString login, CString pass, CString db);
	bool OpenDemandConnection(CString ip, CString login, CString pass, CString db);
	bool OpenSupplyConnection();
	bool OpenDemandConnection();
	bool OpenSupplyHashConnection(CString ip, CString login, CString pass, CString db);
	void CloseDemandConnection();
	void CloseSupplyConnection();
	int InsertRawGnutellaSupply(vector<QueryHit>& query_hits);
	int InsertRawGnutellaSupply(/*CString& file_name,UINT file_size, */
			int ip, CString& timestamp, char* sha1, /*GUID& guid,*/ CString& project,int track/*,bool spoof*/);
	//int InsertRawGnutellaDemand(CString& project, int track, CString& query, CString& timestamp);
	int InsertRawGnutellaDemand(vector<Query>& project_queries);
	int InsertGUID(char* table, GUID& guid, CString& project, const char* timestamp);
//	bool ReleaseLock();
	int CreateProjectDemandTable(CString& project_demand_table);
	int CreateProjectSupplyTable(CString& project_supply_table);

	MYSQL *p_supply_conn;
	MYSQL *p_demand_conn;
	MYSQL *p_supply_hash_conn;
	UINT GetGUIDIndex(GUID& guid);

	CString m_supply_db_address;
	CString m_supply_db_user;
	CString m_supply_db_password;
	CString m_supply_db_name;

	CString m_demand_db_address;
	CString m_demand_db_user;
	CString m_demand_db_password;
	CString m_demand_db_name;
};