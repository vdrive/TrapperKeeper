#pragma once

#include "mysql.h"
#include "stdafx.h"

class SQLInterface
{
public:
	SQLInterface(void);
	~SQLInterface(void);

	string EscapedString(string input);
	bool Execute(MYSQL * conn, string query);
	MYSQL_RES * Query(MYSQL * conn, string query);
	bool OpenDB(MYSQL *, const char * address, const char * database);
	bool OpenDB(MYSQL *conn, const char * address, const char * user, const char * password, const char * database);
	void CloseDB(MYSQL *);

protected:
	const char * m_address;
	const char * m_database;
};
