#include "StdAfx.h"
#include "databasehelper.h"

DatabaseHelper::DatabaseHelper(void)
{
	m_conn = NULL;
	m_res = NULL;
	b_init = false;
	//m_last_error_msg[0] = '\0';
	m_num_rows = -1;
}

DatabaseHelper::~DatabaseHelper(void)
{
	if(m_conn != NULL || m_res != NULL) CloseDB();
}

void DatabaseHelper::Init(const char *address, const char * user, const char * password, const char * database) 
{
	m_address = address;
	m_user = user;
	m_password = password;
	m_database = database;
	b_init = true;
	OpenDB();
}

bool DatabaseHelper::Execute(const char * query) 
{
	if(!b_init) {
		m_last_error_msg="not initialized";
		return false;
	}
	CSingleLock locked(&m_lock);
	locked.Lock();
	if(locked.IsLocked())
	{
		int ret = mysql_query(m_conn, query);
		if (ret != 0)	// check for error
		{
			string error_message = mysql_error(m_conn);
			if(error_message.find("Lost connection to MySQL server during query") == 0 || 
				error_message.find("MySQL server has gone away") == 0) {
				if(OpenDB()) {
					locked.Unlock();
					return Execute(query);
				}
			}
			m_last_error_msg=mysql_error(m_conn);	
			locked.Unlock();
			return false;
		}
		else
		{
			locked.Unlock();
			return true;
		}
	}
	return false;
}

bool DatabaseHelper::Query(const char * query) 
{
	if(!b_init) {
		m_last_error_msg="not initialized";
		return false;
	}
	CSingleLock locked(&m_lock);
	locked.Lock();
	if(locked.IsLocked())
	{
		int ret = mysql_query(m_conn, query);
		if (ret != 0)	{ // check for error
			string error_message = mysql_error(m_conn);
			if(error_message.find("Lost connection to MySQL server during query") == 0 || 
				error_message.find("MySQL server has gone away") == 0) {
				if(OpenDB()) {
					locked.Unlock();
					return Query(query);
				}
			}
			m_last_error_msg=mysql_error(m_conn);
			locked.Unlock();
			return false;
		}
		if(m_res != NULL)
		{
			mysql_free_result(m_res);			// free's mem (::)
			m_res = NULL;
		}
		m_res = mysql_store_result(m_conn); // allocates mem (::)

		if(m_res != NULL)
		{
			m_num_rows = (int)mysql_num_rows(m_res);
			locked.Unlock();
			return true;

		}
		locked.Unlock();
	}
	return false;
}

MYSQL_ROW DatabaseHelper::GetNextRow()
{
	return mysql_fetch_row(m_res);
}

int DatabaseHelper::GetNumRows() 
{
	return m_num_rows;
}

bool DatabaseHelper::OpenDB() 
{
	if(!b_init) {
		m_last_error_msg="not initialized";
		return false;
	}
	if(m_conn!=NULL)
	{
		CloseDB();
	}

	m_conn = new MYSQL();

	m_conn->reconnect = true;
	mysql_init(m_conn); 
	if (mysql_real_connect(m_conn,m_address.c_str(),m_user.c_str(),m_password.c_str(),m_database.c_str(),0,NULL,0) == NULL)
	{
		m_last_error_msg.Format("Failed to connect to database: %s", mysql_error(m_conn));

		return false;
	}
	TRACE("DatabaseHelper::OpenDB  DB Open\n");
	return true;
}

void DatabaseHelper::CloseDB() 
{
	if(m_res != NULL) {
		mysql_free_result(m_res);			// free's mem (::)
		m_res = NULL;
	}
	if(m_conn != NULL) {
		mysql_close(m_conn);
		delete m_conn;
		m_conn = NULL;
	}
}

CString DatabaseHelper::GetLastErrorMessage()
{
	return m_last_error_msg;
}

bool DatabaseHelper::IsConnected()
{
	return (m_conn != NULL);
}