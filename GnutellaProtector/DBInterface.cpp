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
		/*
		CString errmsg = "Failed to connect to database: Error: ";
		errmsg += mysql_error(p_conn);
		TRACE(errmsg);
		MessageBox(NULL,errmsg,"Error",MB_OK|MB_ICONERROR);
		*/
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " DBInterface::OpenConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";
			file.WriteString(log);
			file.Close();
		}
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
int DBInterface::InsertGnutellaSPD(char* table, CString project, int track, UINT spoofs, UINT poisons,UINT dists, CString& timestamp)
{
	char*  escape_string = new char[strlen(project)*2+1];
	memset(escape_string,0,strlen(project)*2+1);
	UINT project_size = mysql_real_escape_string(p_conn, escape_string, project, (UINT)strlen(project));

	int ret;
	char temp[32];
	memset(temp, 0, sizeof(temp));

	CString query_str = "INSERT INTO ";
	query_str += table;
	query_str += " (project,track,spoofs,poisons,dists,timestamp) VALUES ('";

	query_str += escape_string;
	query_str += "',";
	query_str += itoa(track, temp, 10);
	query_str += ",";
	query_str += ultoa(spoofs, temp, 10);
	query_str += ",";
	query_str += ultoa(poisons, temp, 10);
	query_str += ",";
	query_str += ultoa(dists, temp, 10);
	query_str += ",'";
	query_str += timestamp;
	query_str += "')";

	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertGnutellaSPF(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	delete [] escape_string;
	return ret;
}
