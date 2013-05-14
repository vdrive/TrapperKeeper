#include "StdAfx.h"
#include "processdbinterface.h"
#include "TrackSupply.h"

ProcessDBInterface::ProcessDBInterface(void)
{
}

//
//
//
ProcessDBInterface::~ProcessDBInterface(void)
{
}

//
//
//
bool ProcessDBInterface::OpenRawDataConnection(CString ip, CString login, CString pass, CString db)
{
	p_raw_data_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_raw_data_conn, ip, login, pass, db, 0, NULL, 0))
	{
		CString errmsg = "Failed to connect to raw database: Error: ";
		errmsg += mysql_error(p_raw_data_conn);
		TRACE(errmsg);
#ifdef _DEBUG
		MessageBox(NULL,errmsg,"Error",MB_OK|MB_ICONERROR);
#endif
		return false;
	}
	return true;
}

//
//
//
void ProcessDBInterface::CloseRawDataConnection()
{
	mysql_close(p_raw_data_conn);
}

//
//
//
int ProcessDBInterface::GetLastProcessedDateForSpoof(CString project, char* date)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT MAX(date) from piolet_spoofs where project='";
	query_str += project;
	query_str += "'";

	ret = mysql_query(p_conn, query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetLastProcessedDateForSpoof(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
#endif
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret==0)
	{
		MYSQL_RES *res=NULL;
		MYSQL_ROW row;

		res=mysql_store_result(p_conn);	// allocates memory
		if(res != NULL)
		{
			UINT num_row = (UINT)mysql_num_rows(res);
			if(num_row > 0)
			{
				row = mysql_fetch_row(res);
				if(row[0] != NULL)
					strcpy(date, row[0]);
				else
					date = NULL;
			}
			else
				date=NULL;
		}
		else
			date = NULL;

		mysql_free_result(res);	// free memory
	}
	return ret;
}

//
//
//
int ProcessDBInterface::GetFirstSpoofDataInsertionTimestamp(CString project, char* timestamp)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT MIN(timestamp) from piolet_raw_spoofs where project='";
	query_str += project;
	query_str += "'";

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetFirstSpoofDataInsertionTimestamp(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_raw_data_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
#ifdef _DEBUG
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
#endif
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret==0)
	{
		MYSQL_RES *res=NULL;
		MYSQL_ROW row;

		res=mysql_store_result(p_raw_data_conn);	// allocates memory
		if(res != NULL)
		{
			UINT num_row = (UINT)mysql_num_rows(res);
			if(num_row > 0)
			{
				row = mysql_fetch_row(res);
				if(row[0] != NULL)
					strcpy(timestamp, row[0]);
				else
					timestamp = NULL;
			}
			else
				timestamp = NULL;
		}
		else
			timestamp = NULL;

		mysql_free_result(res);	// free memory
	}
	return ret;
}


//
//
//
bool ProcessDBInterface::ProcessSpoof(CString project, char* start, char* end, char* on_date)
{
	bool inserted_result = false;
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT SUM(spoofs) from piolet_raw_spoofs where project='";
	query_str += project;
	query_str += "' and timestamp >= '";
	query_str += start;
	query_str += "' and timestamp <= '";
	query_str += end;
	query_str += "'";

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ProcessSpoof(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_raw_data_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
#ifdef _DEBUG
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
#endif
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret == -1)
		return true;

	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	res=mysql_store_result(p_raw_data_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL)
				{
					InsertProcessedSpoofData(project, on_date, row[0]);
					inserted_result = true;
				}
			}
		}
	}

	mysql_free_result(res);	// free memory
	return inserted_result;
}

//
//
//
int ProcessDBInterface::InsertProcessedSpoofData(CString project, char* date, const char* spoofs)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "INSERT INTO piolet_spoofs";
	query_str += " (date,project,spoofs) VALUES ('";
	query_str += date;
	query_str += "','";
	query_str += project;
	query_str += "',";
	query_str += spoofs;
	query_str += ")";

	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertProcessedSpoofData(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
#endif
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	return ret;
}

//
//
//
UINT ProcessDBInterface::DeleteRawData(const char* timestamp, const char* table)
{
	int ret;

	CString query_str = "DELETE FROM ";
	query_str += table;
	query_str += " where timestamp < '";
	query_str += timestamp;
	query_str += "'";

	ret = mysql_query(p_raw_data_conn,query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " DeleteRawData(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_raw_data_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
#ifdef _DEBUG
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
#endif
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret == -1)
		return 0;
	
	UINT delete_records = (UINT)mysql_affected_rows(p_raw_data_conn);
	return delete_records;
}

//
//
//
int ProcessDBInterface::OptimizeTables(vector<CString>& tables)
{
	int ret;

	CString query_str = "OPTIMIZE TABLE ";
	for(UINT i=0; i < tables.size(); i++)
	{
		if(i!=0)
			query_str += ",";
		query_str += tables[i];
	}

	ret = mysql_query(p_raw_data_conn,query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " OptimizeTables(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_raw_data_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
#ifdef _DEBUG
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
#endif
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	return ret;
}
