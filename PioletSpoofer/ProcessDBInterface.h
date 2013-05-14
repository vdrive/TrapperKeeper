#pragma once
#include "dbinterface.h"

class ProcessDBInterface : public DBInterface
{
public:
	ProcessDBInterface(void);
	~ProcessDBInterface(void);
	bool OpenRawDataConnection(CString ip, CString login, CString pass, CString db);
	void CloseRawDataConnection();
//	void GetAllDemandProjects(vector<CString>& projects);
//	void GetAllSupplyProjects(vector<CString>& projects);
	int GetLastProcessedDateForSpoof(CString project, char* date);
	int GetFirstSpoofDataInsertionTimestamp(CString project, char* timestamp);
	bool ProcessSpoof(CString project, char* start, char* end, char* on_date);
	int InsertProcessedSpoofData(CString project, char* date, const char* spoofs);

	//returns number of records deleted
	UINT DeleteRawData(const char* timestamp, const char* table);
	int OptimizeTables(vector<CString>& tables);


	
	MYSQL *p_raw_data_conn;
};
