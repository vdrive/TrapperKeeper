// DBInterface.h
#pragma once

#include "mysql.h"
#include "QueryHit.h"
#include "VirtualProjectStatusListCtrlItem.h"

class DBInterface  
{
public:
	DBInterface();
	virtual ~DBInterface();

	bool OpenSupplyConnection(CString ip, CString login, CString pass, CString db);
	void CloseSupplyConnection();
	int InsertRawFastTrackSupply(vector<QueryHit>& query_hits);
	int InsertRawFastTrackSupply(QueryHit& query_hits);
	int InsertDistinctFastTrackSupply(vector<QueryHit>& query_hits,const char* today_date);
	int InsertDistinctFastTrackSupply(QueryHit& query_hits,const char* today_date);
	int CreateProjectSupplyTable(CString& project_supply_table);
	CString CreateFakeSupplyTable(const char* project);
	int CreateDistinctProjectSupplyTable(CString& project_supply_table);
	int InsertFakeSupplies(VirtualProjectStatusListCtrlItem& data);

	MYSQL *p_supply_conn;
};