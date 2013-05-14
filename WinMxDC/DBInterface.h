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
	int InsertRawSupply(vector<QueryHit>& query_hits);
	int InsertRawSupply(QueryHit& query_hits);
	int CreateProjectSupplyTable(CString& project_supply_table);
	//CString CreateFakeSupplyTable(const char* project);
	//int InsertFakeSupplies(VirtualProjectStatusListCtrlItem& data);

	MYSQL *p_supply_conn;
};