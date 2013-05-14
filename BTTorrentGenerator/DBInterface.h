#pragma once
#include "StdAfx.h"
#include "MySQL.h"

class DBInterface  
{
public:
	DBInterface();
	~DBInterface();

	bool OpenConnection(CString ip, CString login, CString pass, CString db);
	void CloseConnection();
	int DBInterface::executeQuery(string query);
	MYSQL * getConnectionHandle();
	MYSQL * conn;
};
