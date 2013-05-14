#include "sqlinterface.h"

SQLInterface::SQLInterface(void)
{
}

SQLInterface::~SQLInterface(void)
{
}

bool SQLInterface::Execute(MYSQL * conn, string query)
{
	int ret = mysql_query(conn, query.c_str());
	if (ret != 0)	{// check for error
		TRACE(mysql_error(conn));
		TRACE("\nSQLInterface::Execute(Query: %s)\n", query.c_str());
		return false;
	}
	return true;
}

MYSQL_RES * SQLInterface::Query(MYSQL * conn, string query)
{
	MYSQL_RES *res;
	int ret = mysql_query(conn, query.c_str());
	if (ret != 0)	{// check for error
		TRACE(mysql_error(conn));
		TRACE("\nSQLInterface::Execute(Query: %s)\n", query.c_str());
		return NULL;
	}
	res = mysql_store_result(conn); // allocates mem (::)
	return res;
}

string SQLInterface::EscapedString(string input) 
{
	CString source = input.c_str();
	source.Replace("\\", "\\\\");
	source.Replace("'", "\\'");
	//TRACE("pre: %s, post: %s\n", input.c_str(), source.GetBuffer());
	return source.GetBuffer();
}

bool SQLInterface::OpenDB(MYSQL *conn, const char * address, const char * database) 
{
	return OpenDB(conn, address, "onsystems", "ebertsux37", database);
}

bool SQLInterface::OpenDB(MYSQL *conn, const char * address, const char * user, const char * password, const char * database) 
{
	m_address = address;
	m_database = database;
	TRACE("OpenDB, %s, %s\n", m_address, m_database);
	mysql_init(conn);
	if (mysql_real_connect(conn,address,user,password,database,3306,NULL,0) == NULL)
	{
		TRACE("SQLInterface::OpenDB\n(Failed to connect to database: %s)\n", mysql_error(conn));
		return false;
	}
	conn->reconnect= 1;
	return true;
}

void SQLInterface::CloseDB(MYSQL *rs) 
{
	mysql_close(rs);
}
