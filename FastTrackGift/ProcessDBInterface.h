#pragma once
#include "dbinterface.h"

class ProcessDBInterface : public DBInterface
{
public:
	ProcessDBInterface(void);
	~ProcessDBInterface(void);
	bool OpenRawDataConnection(CString ip, CString login, CString pass, CString db);
	void CloseRawDataConnection();
	int GetLastProcessedDateForSupply(CString project, char* date);
	int GetFirstSupplyDataInsertionTimestamp(CString project, char* timestamp);
	bool ProcessSupply(CString project, char* start, char* end, char* on_date, int total_tracks);
	int InsertProcessedSupplyData(CString project, char* date, const char* track, const char* supply);
	int InsertProcessedFakeSupplyData(CString project, char* date, const char* track, const char* supply);

	//returns number of records deleted
	UINT DeleteRawData(const char* timestamp, const char* table);
	int OptimizeTables(vector<CString>& tables);
	int OptimizeTable(CString& table);

	int	GetAllSupplyTables(vector<CString>& tables);
	int DeleteTable(const char* table);

	MYSQL *p_raw_data_conn;
};
