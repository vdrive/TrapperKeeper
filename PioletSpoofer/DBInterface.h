// DBInterface.h
#pragma once

#include "mysql.h"
#include "Query.h"

class DBInterface  
{
public:
	DBInterface();
	virtual ~DBInterface();

	bool OpenConnection(CString ip, CString login, CString pass, CString db);
	void CloseConnection();
	int InsertRawPioletSpoof(char* table, CString project, Query* query, CString& timestamp);
	bool ReleaseLock();

	MYSQL *p_conn;
};