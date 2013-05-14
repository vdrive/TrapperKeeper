// DBInterface.cpp: implementation of the DBInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DBInterface.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DBInterface::DBInterface()
{

}

DBInterface::~DBInterface()
{

}

bool DBInterface::OpenConnection(CString ip, CString login, CString pass, CString db)
{
	p_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_conn, ip, login, pass, db, 0, NULL, 0))
	{
		CString errmsg = "Failed to connect to database: Error: ";
		errmsg += mysql_error(p_conn);
		TRACE(errmsg);
#ifdef _DEBUG
		MessageBox(NULL,errmsg,"Error",MB_OK|MB_ICONERROR);
#endif
		return false;
	}
	return true;
}

void DBInterface::CloseConnection()
{
	mysql_close(p_conn);
}

//
//	Releases the processing_lock
//	Returns true if the lock was released properly
//
bool DBInterface::ReleaseLock()
{
	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	bool ret = false;

	mysql_query(p_conn, "SELECT RELEASE_LOCK(\"processing_lock\")");
	res=mysql_store_result(p_conn);
	row=mysql_fetch_row(res);

	if(strcmp(row[0],"1")==0)
		ret = true;

	mysql_free_result(res);

	return ret;
}

//
//
//
int DBInterface::InsertRawPioletSpoof(char* table, CString project, Query* query, CString& timestamp)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	CString csQuery = query->m_query.c_str();
	csQuery.Replace("\\","_");

	project.Replace("\'", "\\\'");		// replace the single quote "'"
	csQuery.Replace("\'", "\\\'");

	int ret;
	char temp[32];
	memset(temp, 0, sizeof(temp));

	CString query_str = "INSERT INTO ";
	query_str += table;
	query_str += " (project,timestamp,ip,query,track,spoofs) VALUES ('";

	query_str += project;
	query_str += "','";
	query_str += timestamp;
	query_str += "',";
	query_str += ultoa(query->m_ip, temp, 10);
	query_str += ",'";
	query_str += csQuery;
	query_str += "',";
	query_str += itoa(query->m_track,temp,10);
	query_str += ",";
	query_str += ultoa(query->m_spoofs_sent, temp, 10);	
	query_str += ")";

	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertRawPioletSpoof(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
#ifdef _DEBUG
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
#endif
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	return ret;
}