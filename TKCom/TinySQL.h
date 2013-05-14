#pragma once

#ifndef FD_SET  //if not already using a sockets implementation, then use winsock2
#include <winsock2.h>
#endif

#include "mysql.h"

class TinySQL
{
public:
	MYSQL *mp_sql_reference;  //really a MYSQL data type.
	MYSQL_RES	* mp_sql_results ;
	string **mpp_results;  //string[row][column]
	int m_fail_code;
	string m_fail_reason;
	UINT m_num_rows;
	UINT m_num_fields;
	string m_init_host;
	string m_init_login;
	string m_init_password;
	string m_init_database;
	UINT m_init_port;
	TinySQL(void);
	~TinySQL(void);
	bool Init(const char* host, const char* login, const char* password, const char* database, UINT port);
	bool Query(const char* query,bool b_store_results);
	void ClearResults(void);
	void Reset(void);

	void LogError(const char* error);
};
