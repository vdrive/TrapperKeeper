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
	int InsertRawPioletSupply(char* table,
							CString& project,
							UINT ip,
							CString& nickname,
							UINT file_size,
							int track,
							int bitrate,
							int sampling_rate,
							int stereo,
							int song_length,
							CString& filename,
							char* md5,
							CString& timestamp,
							UINT header_counter);

	int InsertRawPioletDemand(char* table, CString project, int track, CString query, CString& timestamp, UINT ip);
//	int InsertGUID(char* table, GUID& guid, CString& project, const char* timestamp);
	bool ReleaseLock();

	MYSQL *p_conn;
//	UINT GetGUIDIndex(GUID& guid);
};