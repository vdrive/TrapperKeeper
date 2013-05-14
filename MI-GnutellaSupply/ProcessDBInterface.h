#pragma once
#include "dbinterface.h"
#include "ProjectKeywords.h"

class ProcessDBInterface : public DBInterface
{
public:
	ProcessDBInterface(void);
	~ProcessDBInterface(void);
	bool OpenRawDataConnection(CString ip, CString login, CString pass, CString db);
	bool OpenRawDataConnection();
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
	int DeleteProcessedIPs(const char* table);
	int OptimizeProcessedIPs(ProjectKeywords& project);
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
	bool	GetAllProcessedSupplyTables(vector<CString>& tables);
	UINT DeleteOldHashes();
	UINT UpdateHashTable(UINT& delete_records_from_hash_table);
	int DeleteTable(const char* table);

	bool ProcessRealTimeSupply(ProjectKeywords& project);
	bool InsertSupplyIPResultsToProcessedDB(vector<int>&IPs, vector<string>& sha1s, const char* artist_name, 
		const char* album_name=NULL, const char* track_name=NULL);
	bool InsertSupplyResultsToProcessedDB(const char* artist, const char* album, const char* track, UINT value);
	bool DeleteAllRawData(const char* project);
	bool CreateDBTable(const char* table, CString& new_table_name);

	MYSQL *p_raw_data_conn;

	CString m_raw_db_address;
	CString m_raw_db_user;
	CString m_raw_db_password;
	CString m_raw_db_name;
	

};
