#pragma once
#include "mysql.h"	// for database access (also need libmysql.lib)
class DB
{
public:
	DB(void);
	~DB(void);
	bool CreateDatabase();	//create database
	bool CreateTables();	//create tables
	bool Connect(CString ip, CString login, CString pass, CString db);
	void Disconnect();	//disconnect user from database
	int InsertData(CString project,CString track_name,int track,CString file_name,CString file_size, CString user,CString ip_address,CString table);	//inserts data into table
	void InsertDCMasterData();
private:
	MYSQL *p_conn;	//connect to local
	char m_track_number[30];
};
