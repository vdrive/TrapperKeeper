#pragma once

#include "mysql.h"
#include "afxmt.h"

class DatabaseHelper
{
public:
	DatabaseHelper(void);
	~DatabaseHelper(void);

	bool OpenDB();

	CString GetLastErrorMessage();
	void Init(const char *address, const char * user, const char * password, const char * database);

	bool Execute(const char * query);
	bool Query(const char * query);
	bool IsConnected();
	int GetNumRows();
	MYSQL_ROW GetNextRow();
protected:
	void CloseDB();

	MYSQL *m_conn;
	MYSQL_RES *m_res;
	string m_address;
	string m_user;
	string m_password;
	string m_database;

	bool b_init;
	CString m_last_error_msg;
	int m_num_rows;

	CCriticalSection m_lock;
};
