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
	int InsertProcessedDemandData(CString project, char* date, const char* track, const char* demand);
	bool ProcessSupply(CString project, char* start, char* end, char* on_date);
	int InsertProcessedSupplyData(CString project, char* date, const char* track, const char* supply);
	//returns number of records deleted
	UINT DeleteRawData(const char* timestamp, const char* table);
	int OptimizeTables(vector<CString>& tables);
	int OptimizeTable(CString& table);

	int ProcessReverseDNS(CString project, char* start, char* end, char* on_date);
	int GetLastProcessedDateForReverseDNS(CString project, char* date);
	int GetFirstReverseDNSDataInsertionTimestamp(CString project, char* timestamp);
	int InsertProcessedReverseDNSData(CString project, char* date, vector<CString>& IPs);
	
	bool	GetAllDemandTables(vector<CString>& tables);
	bool	GetAllSupplyTables(vector<CString>& tables);
	UINT DeleteOldHashes();
	UINT UpdateHashTable(UINT& delete_records_from_hash_table);
	int DeleteTable(const char* table);

	MYSQL *p_raw_data_conn;
};
