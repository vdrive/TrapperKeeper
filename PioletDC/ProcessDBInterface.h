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
	int GetLastProcessedDateForDemand(CString project, char* date);
	int GetLastProcessedDateForSupply(CString project, char* date);
	int GetFirstDemandDataInsertionTimestamp(CString project, char* timestamp);
	int GetFirstSupplyDataInsertionTimestamp(CString project, char* timestamp);
	bool ProcessDemand(CString project, char* start, char* end, char* on_date);
	int InsertProcessedDemandData(CString project, char* date, const char* track, const char* demand, const char* unique_user);
	bool ProcessSupply(CString project, char* start, char* end, char* on_date);
	int InsertProcessedSupplyData(CString project, char* date, const char* track, const char* supply);
	//returns number of records deleted
	UINT DeleteRawData(const char* timestamp, const char* table);
	int OptimizeTables(vector<CString>& tables);


	
	MYSQL *p_raw_data_conn;
};
