#pragma once
#include "mysql.h"	// for database access (also need libmysql.lib)
#include "SpoofStruct.h"
class DB
{
public:
	DB(void);
	~DB(void);
	bool CreateDatabase();	//create database
	bool CreateTables();	//create tables
	bool Connect(CString ip, CString login, CString pass, CString db);		//connects to database
	void Disconnect();	//disconnects user from database
	int InsertData(CString project,CString track_name,CString file_name,CString file_size, CString user,CString ip_address,CString table);	//inserts data into table
	int ExtractData(CString project,CString track_name,CString table, vector<SpoofStruct> &v_spoofs);	//extract data from database to spoof
	void DataExists(string project);	//checks whether project has data
	void SingleTrackDataExists(string project,int track);
	void IncrementDBSpoofs(string project,int spoofs);		//increment spoofs
	void IncrementDBDecoys(string project,int decoys);		//increment decoys
	void IncrementDBRequests(string project,int requests,int track);	//increment requests
	string GetDate();		//gets the current date
	void InsertDCMasterData();		//inserts data into the dcdata table
	string GetFilesize(string filename);
private:
	MYSQL *p_conn;
};
