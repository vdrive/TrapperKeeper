// DBInterface.h
#pragma once

#include "mysql.h"

class DBInterface  
{
public:
	DBInterface();
	virtual ~DBInterface();

	bool OpenConnection(CString ip, CString login, CString pass, CString db);
	void CloseConnection();
//	int InsertRawGnutellaSupply(char* table, CString& file_name,UINT file_size, 
//			int ip, CString& timestamp, char* sha1, GUID& guid, CString& project,int track,bool spoof);
//	int InsertRawGnutellaDemand(char* table, CString project, int track, CString query, CString& timestamp);
	int InsertGnutellaSPD(char* table, CString project, int track, UINT spoofs, UINT poisons,UINT dists, CString& timestamp);
//	int InsertGUID(char* table, GUID& guid, CString& project, const char* timestamp);
	bool ReleaseLock();

	MYSQL *p_conn;
//	UINT GetGUIDIndex(GUID& guid);
};