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
	m_raw_db_address=ip;
	m_raw_db_user=login;
	m_raw_db_password=pass;
	m_raw_db_name=db;

	p_raw_data_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_raw_data_conn, ip, login, pass, db, 0, NULL, 0))
	{
		/*
		CString errmsg = "Failed to connect to raw database: Error: ";
		errmsg += mysql_error(p_raw_data_conn);
		TRACE(errmsg);
		MessageBox(NULL,errmsg,"Error",MB_OK|MB_ICONERROR);
		*/
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ProcessDBInterface::OpenConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_raw_data_conn);
			log += "\n\n";
			file.WriteString(log);
			file.Close();
		}
		return false;
	}
	return true;
}

//
//
//
bool ProcessDBInterface::OpenRawDataConnection()
{
	CloseRawDataConnection();
	p_raw_data_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_raw_data_conn, m_raw_db_address, m_raw_db_user, m_raw_db_password, m_raw_db_name, 0, NULL, 0))
	{
		/*
		CString errmsg = "Failed to connect to raw database: Error: ";
		errmsg += mysql_error(p_raw_data_conn);
		TRACE(errmsg);
		MessageBox(NULL,errmsg,"Error",MB_OK|MB_ICONERROR);
		*/
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ProcessDBInterface::OpenConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_raw_data_conn);
			log += "\n\n";
			file.WriteString(log);
			file.Close();
		}
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
/*
//
//
//
void ProcessDBInterface::GetAllDemandProjects(vector<CString>& projects)
{
	int ret;

	CString query_str = "SELECT project from gnutella_raw_demand group by project";

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetAllDemandProjects(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	res=mysql_store_result(p_raw_data_conn);	// allocates memory
	if(res != NULL)
	{
		while ((row = mysql_fetch_row(res)))
		{
			CString project = row[0];
			projects.push_back(project);
		}
	}


	mysql_free_result(res);	// free memory
}

//
//
//
void ProcessDBInterface::GetAllSupplyProjects(vector<CString>& projects)
{
	int ret;

	CString query_str = "SELECT project from gnutella_raw_supply group by project";

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetAllSupplyProjects(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	res=mysql_store_result(p_raw_data_conn);	// allocates memory
	if(res != NULL)
	{
		while ((row = mysql_fetch_row(res)))
		{
			CString project = row[0];
			projects.push_back(project);
		}
	}


	mysql_free_result(res);	// free memory
}
*/

//
//
//
int ProcessDBInterface::GetLastProcessedDateForDemand(CString project, char* date)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT MAX(date) from gnutella_demand where project='";
	query_str += project;
	query_str += "'";

	ret = mysql_query(p_demand_conn, query_str);
	
	if(mysql_error(p_demand_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetLastProcessedDateForDemand(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_demand_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_demand_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret==0)
	{
		MYSQL_RES *res=NULL;
		MYSQL_ROW row;

		res=mysql_store_result(p_demand_conn);	// allocates memory
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
int ProcessDBInterface::GetLastProcessedDateForReverseDNS(CString project, char* date)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT MAX(time) from gnutella_network_info where project='";
	query_str += project;
	query_str += "'";

	ret = mysql_query(p_supply_conn, query_str);
	
	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetLastProcessedDateForDemand(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret==0)
	{
		MYSQL_RES *res=NULL;
		MYSQL_ROW row;

		res=mysql_store_result(p_supply_conn);	// allocates memory
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
int ProcessDBInterface::GetLastProcessedDateForSupply(CString project, char* date)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT MAX(date) from gnutella_supply where project='";
	query_str += project;
	query_str += "'";

	ret = mysql_query(p_supply_conn, query_str);
	
	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetLastProcessedDateForSupply(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret==0)
	{
		MYSQL_RES *res=NULL;
		MYSQL_ROW row;

		res=mysql_store_result(p_supply_conn);	// allocates memory
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
				date = NULL;
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
int ProcessDBInterface::GetFirstDemandDataInsertionTimestamp(CString project, char* timestamp)
{
	/*
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"
	*/

	//truncate the project name if its length is greater than 50
	if(project.GetLength() > 50)
	{
		project.Truncate(50);
	}

	CString table = "DEMAND_TABLE_";
	table += project;
	table.Replace('\\','_');			// replace the backslash with _
	table.Replace('\'', '_');		// replace the single quote "'" with _
	table.Replace(' ', '_');
	table.Replace('-', '_');
	table.Replace('&', '_');
	table.Replace('!', '_');
	table.Replace('$', '_');
	table.Replace('@', '_');
	table.Replace('%', '_');
	table.Replace('(', '_');
	table.Replace(')', '_');
	table.Replace('+', '_');
	table.Replace('~', '_');
	table.Replace('*', '_');
	table.Replace('.', '_');
	table.Replace(',', '_');
	table.Replace('?', '_');
	table.Replace(':', '_');
	table.Replace(';', '_');
	table.Replace('"', '_');
	table.Replace('/', '_');
	table.Replace('#', '_');

	int ret;

	CString query_str = "SELECT MIN(timestamp) from ";
	query_str += table;

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetFirstDemandDataInsertionTimestamp(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
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
int ProcessDBInterface::GetFirstReverseDNSDataInsertionTimestamp(CString project, char* timestamp)
{
//	project.Replace("\\","_");			// replace the backslash with a "_"
//	project.Replace("\'", "\\\'");		// replace the single quote "'"

	//truncate the project name if its length is greater than 50
	if(project.GetLength() > 50)
	{
		project.Truncate(50);
	}

	CString table = "SUPPLY_TABLE_";
	table += project;
	table.Replace('\\','_');			// replace the backslash with _
	table.Replace('\'', '_');		// replace the single quote "'" with _
	table.Replace(' ', '_');
	table.Replace('-', '_');
	table.Replace('&', '_');
	table.Replace('!', '_');
	table.Replace('$', '_');
	table.Replace('@', '_');
	table.Replace('%', '_');
	table.Replace('(', '_');
	table.Replace(')', '_');
	table.Replace('+', '_');
	table.Replace('~', '_');
	table.Replace('*', '_');
	table.Replace('.', '_');
	table.Replace(',', '_');
	table.Replace('?', '_');
	table.Replace(':', '_');
	table.Replace(';', '_');
	table.Replace('"', '_');
	table.Replace('/', '_');
	table.Replace('#', '_');
	int ret;

	CString query_str = "SELECT MIN(timestamp) from ";
	query_str += table;

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetFirstReverseDNSDataInsertionTimestamp(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
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
int ProcessDBInterface::GetFirstSupplyDataInsertionTimestamp(CString project, char* timestamp)
{
//	project.Replace("\\","_");			// replace the backslash with a "_"
//	project.Replace("\'", "\\\'");		// replace the single quote "'"
	CString table = "SUPPLY_TABLE_";

	//truncate the project name if its length is greater than 50
	if(project.GetLength() > 50)
	{
		project.Truncate(50);
	}

	table += project;
	table.Replace('\\','_');			// replace the backslash with _
	table.Replace('\'', '_');		// replace the single quote "'" with _
	table.Replace(' ', '_');
	table.Replace('-', '_');
	table.Replace('&', '_');
	table.Replace('!', '_');
	table.Replace('$', '_');
	table.Replace('@', '_');
	table.Replace('%', '_');
	table.Replace('(', '_');
	table.Replace(')', '_');
	table.Replace('+', '_');
	table.Replace('~', '_');
	table.Replace('*', '_');
	table.Replace('.', '_');
	table.Replace(',', '_');
	table.Replace('?', '_');
	table.Replace(':', '_');
	table.Replace(';', '_');
	table.Replace('"', '_');
	table.Replace('/', '_');
	table.Replace('#', '_');
	int ret;

	CString query_str = "SELECT MIN(timestamp) from ";
	query_str += table;

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetFirstSupplyDataInsertionTimestamp(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
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
bool ProcessDBInterface::ProcessDemand(CString project, char* start, char* end, char* on_date)
{
	bool inserted_result = false;
/*
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"
*/

	//truncate the project name if its length is greater than 50
	CString truncated_project = project;
	if(truncated_project.GetLength() > 50)
	{
		truncated_project.Truncate(50);
	}

	CString table = "DEMAND_TABLE_";
	table += truncated_project;
	table.Replace('\\','_');			// replace the backslash with _
	table.Replace('\'', '_');		// replace the single quote "'" with _
	table.Replace(' ', '_');
	table.Replace('-', '_');
	table.Replace('&', '_');
	table.Replace('!', '_');
	table.Replace('$', '_');
	table.Replace('@', '_');
	table.Replace('%', '_');
	table.Replace('(', '_');
	table.Replace(')', '_');
	table.Replace('+', '_');
	table.Replace('~', '_');
	table.Replace('*', '_');
	table.Replace('.', '_');
	table.Replace(',', '_');
	table.Replace('?', '_');
	table.Replace(':', '_');
	table.Replace(';', '_');
	table.Replace('"', '_');
	table.Replace('/', '_');
	table.Replace('#', '_');

	int ret;

	CString query_str = "SELECT track, count(*) from ";
	query_str += table;
	query_str += " where timestamp >= '";
	query_str += start;
	query_str += "' and timestamp <= '";
	query_str += end;
	query_str += "' group by track";

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ProcessDemand(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
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
				if(row[0] != NULL && row[1] != NULL)
				{
					InsertProcessedDemandData(project, on_date, row[0], row[1]);
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
int ProcessDBInterface::InsertProcessedDemandData(CString project, char* date, const char* track, const char* demand)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "INSERT INTO gnutella_demand";
	query_str += " (date,project,track,demand) VALUES ('";
	query_str += date;
	query_str += "','";
	query_str += project;
	query_str += "',";
	query_str += track;
	query_str += ",";
	query_str += demand;
	query_str += ")";

	ret = mysql_query(p_demand_conn,query_str);
	
	if(mysql_error(p_demand_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertProcessedDemandData(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_demand_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_demand_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	return ret;
}

//
//
//
bool ProcessDBInterface::ProcessSupply(CString project, char* start, char* end, char* on_date)
{
	bool inserted_result=false;

	
	//truncate the project name if its length is greater than 50
	CString truncated_project = project;
	if(truncated_project.GetLength() > 50)
	{
		truncated_project.Truncate(50);
	}

	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	CString table = "SUPPLY_TABLE_";
	table += truncated_project;
	table.Replace('\\','_');			// replace the backslash with _
	table.Replace('\'', '_');		// replace the single quote "'" with _
	table.Replace(' ', '_');
	table.Replace('-', '_');
	table.Replace('&', '_');
	table.Replace('!', '_');
	table.Replace('$', '_');
	table.Replace('@', '_');
	table.Replace('%', '_');
	table.Replace('(', '_');
	table.Replace(')', '_');
	table.Replace('+', '_');
	table.Replace('~', '_');
	table.Replace('*', '_');
	table.Replace('.', '_');
	table.Replace(',', '_');
	table.Replace('?', '_');
	table.Replace(':', '_');
	table.Replace(';', '_');
	table.Replace('"', '_');
	table.Replace('/', '_');
	table.Replace('#', '_');

	int ret;

	CString query_str = "SELECT guid_index from gnutella_guid_index where project='";
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
			log += " ProcessSupply(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret == -1)
		return true;

	vector<CString> guid_indexes;
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
				if(row[0]!=NULL)
					guid_indexes.push_back(row[0]);
			}
		}
	}
	mysql_free_result(res);	// free memory

	vector<TrackSupply> track_supplies;
	for(UINT i=0; i<guid_indexes.size();i++)
	{
		query_str = "SELECT track, count(*) from ";
		query_str += table;
		query_str += " where guid = ";
		query_str += guid_indexes[i];
		query_str += " group by track";

		ret = mysql_query(p_raw_data_conn, query_str);
		
		if(mysql_error(p_raw_data_conn)[0] != '\0')
		{
			CStdioFile file;
			if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();

				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += " ProcessSupply(...) ";
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
				MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
			}

			TRACE(mysql_error(p_raw_data_conn));
			TRACE("\n");
			TRACE(query_str);
			TRACE("\n");
		}

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
					if(row[0]!=NULL && row[1]!=NULL)
					{
						TrackSupply ts;
						ts.m_track = atoi(row[0]);
						sscanf(row[1],"%u", &ts.m_supply);
						track_supplies.push_back(ts);
					}
				}
			}
		}
		mysql_free_result(res);	// free memory
	} //for

	//calculate average supply
	vector<TrackSupply> average_supplies;
	UINT j=0;
	for(i=0; i<track_supplies.size();i++)
	{
		bool found = false;
		for(j=0; j<average_supplies.size();j++)
		{
			if(track_supplies[i].m_track == average_supplies[j].m_track)
			{
				found = true;
				break;
			}
		}
		if(!found)
			average_supplies.push_back(track_supplies[i]);
		else
			average_supplies[j].m_supply += track_supplies[i].m_supply;
	}

	for(i=0; i<average_supplies.size();i++)
	{
		average_supplies[i].m_supply /= (int)guid_indexes.size();
		char track[32];
		char supply[32];
		sprintf(track, "%d", average_supplies[i].m_track);
		sprintf(supply, "%u", average_supplies[i].m_supply);
		InsertProcessedSupplyData(project, on_date, track, supply);
		inserted_result=true;
	}
	return inserted_result;
}

//
//
//
int ProcessDBInterface::InsertProcessedSupplyData(CString project, char* date, const char* track, const char* supply)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "INSERT INTO gnutella_supply";
	query_str += " (date,project,track,supply) VALUES ('";
	query_str += date;
	query_str += "','";
	query_str += project;
	query_str += "',";
	query_str += track;
	query_str += ",";
	query_str += supply;
	query_str += ")";

	ret = mysql_query(p_supply_conn,query_str);
	
	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertProcessedSupplyData(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
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

	CString query_str = "DELETE QUICK FROM ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	return ret;
}

//
//
//
int ProcessDBInterface::ProcessReverseDNS(CString project, char* start, char* end, char* on_date)
{
//	bool inserted_result = false;
// project.Replace("\\","_");			// replace the backslash with a "_"
//	project.Replace("\'", "\\\'");		// replace the single quote "'"

	//truncate the project name if its length is greater than 50
	CString truncated_project = project;
	if(truncated_project.GetLength() > 50)
	{
		truncated_project.Truncate(50);
	}

	CString table = "SUPPLY_TABLE_";
	table += truncated_project;
	table.Replace('\\','_');			// replace the backslash with _
	table.Replace('\'', '_');		// replace the single quote "'" with _
	table.Replace(' ', '_');
	table.Replace('-', '_');
	table.Replace('&', '_');
	table.Replace('!', '_');
	table.Replace('$', '_');
	table.Replace('@', '_');
	table.Replace('%', '_');
	table.Replace('(', '_');
	table.Replace(')', '_');
	table.Replace('+', '_');
	table.Replace('~', '_');
	table.Replace('*', '_');
	table.Replace('.', '_');
	table.Replace(',', '_');
	table.Replace('?', '_');
	table.Replace(':', '_');
	table.Replace(';', '_');
	table.Replace('"', '_');
	table.Replace('/', '_');
	table.Replace('#', '_');

	int ret;

	CString query_str = "SELECT distinct ip from ";
	query_str += table;
	query_str += " where timestamp >= '";
	query_str += start;
	query_str += "' and timestamp <= '";
	query_str += end;
	query_str += "'";
	query_str += " and spoof='N'";

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ProcessReverseDNS(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret == -1)
		return -2; //stop processing

	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	res=mysql_store_result(p_raw_data_conn);	// allocates memory
	vector<CString> IPs;
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL)
				{
					IPs.push_back((const char*)row[0]);
					//InsertProcessedReverseDNSData(project, on_date, row[0]);
					//inserted_result = true;
				}
			}
		}
	}

	mysql_free_result(res);	// free memory
	if(IPs.size()!=0)
	{
		return InsertProcessedReverseDNSData(project, on_date, IPs);
	}
	return 1;
}

//
//
//
int ProcessDBInterface::InsertProcessedReverseDNSData(CString project, char* date, vector<CString>& IPs)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "INSERT INTO gnutella_network_info";
	query_str += " (time,project,ip) VALUES ";

	for(UINT i=0; i<IPs.size();i++)
	{
		query_str += "('";
		query_str += date;
		query_str += "','";
		query_str += project;
		query_str += "',";
		query_str += IPs[i];
		query_str += ")";
		if(i != IPs.size()-1)
			query_str += ",";
	}

	ret = mysql_query(p_supply_conn,query_str);
	
	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertProcessedReverseDNSData(...) ";
			//log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
			if(log.Find("Lost connection")!=-1)
				return -1;
			else
				return -2;
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	return 1;


	query_str = "INSERT INTO gnutella_network_info2";
	query_str += " (time,project,ip) VALUES ";

	for(UINT i=0; i<IPs.size();i++)
	{
		query_str += "('";
		query_str += date;
		query_str += "','";
		query_str += project;
		query_str += "',";
		query_str += IPs[i];
		query_str += ")";
		if(i != IPs.size()-1)
			query_str += ",";
	}

	ret = mysql_query(p_supply_conn,query_str);
	
	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log2.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertProcessedReverseDNSData2(...) ";
			//log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	
}

//
//
//
bool	ProcessDBInterface::GetAllDemandTables(vector<CString>& tables)
{
	int ret;

	CString query_str = "show tables";

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetAllDemandTables(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret == -1)
		return false;

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
					tables.push_back( (const char*)row[0] );
				}
			}
		}
	}

	mysql_free_result(res);	// free memory
	return true;
}

//
//
//
bool	ProcessDBInterface::GetAllSupplyTables(vector<CString>& tables)
{
	int ret;

	CString query_str = "show tables";

	ret = mysql_query(p_raw_data_conn, query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetAllSupplyTables(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret == -1)
		return false;

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
					if(strcmp((const char*)row[0], "gnutella_guid_index")!= 0)
						tables.push_back( (const char*)row[0] );
				}
			}
		}
	}

	mysql_free_result(res);	// free memory
	return true;
}

//
//
//
bool	ProcessDBInterface::GetAllProcessedSupplyTables(vector<CString>& tables)
{
	int ret;

	CString query_str = "show tables";

	ret = mysql_query(p_supply_conn, query_str);
	
	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetAllProcessedSupplyTables(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	if(ret == -1)
		return false;

	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	res=mysql_store_result(p_supply_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL)
				{
						tables.push_back( (const char*)row[0] );
				}
			}
		}
	}

	mysql_free_result(res);	// free memory
	return true;
}

//
//
//
int ProcessDBInterface::OptimizeTable(CString& table)
{
	int ret;

	CString query_str = "OPTIMIZE TABLE ";
	query_str += table;

	ret = mysql_query(p_raw_data_conn,query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " OptimizeTable(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}

	return ret;
}

//
//
//
//
//
//
UINT ProcessDBInterface::DeleteOldHashes()
{
	CTime current_time = CTime::GetCurrentTime();
	CTime delete_from_date =  current_time - CTimeSpan(2,0,0,0);
	char delete_timestamp[32];
	sprintf(delete_timestamp, "%d%.2d%.2d000000", delete_from_date.GetYear(),delete_from_date.GetMonth(),delete_from_date.GetDay());

	CString query_str = "delete quick from gnutella_hash.temp_hash_table where timestamp < '";
	query_str += delete_timestamp;
	query_str +="'";
	int ret = -1;
	int trial = 0;
	while(ret == -1 && trial < 100)
	{
		trial++;
		ret = mysql_query(p_raw_data_conn, query_str);
		if(mysql_error(p_raw_data_conn)[0] != '\0')
		{
			CStdioFile file;
			if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();

				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += " DeleteOldHashes(...) ";
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
				MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
			}

			TRACE(mysql_error(p_raw_data_conn));
			TRACE("\n");
			TRACE(query_str);
			TRACE("\n");
		}
		if(ret==-1)
		{
			Sleep(1000);
			mysql_ping(p_raw_data_conn);
		}
	}
	if(ret == -1)
		return 0;

	return (UINT)mysql_affected_rows(p_raw_data_conn);
}

//
//
//
UINT ProcessDBInterface::UpdateHashTable(UINT& delete_records_from_hash_table)
{
	delete_records_from_hash_table=0;
	int ret;
	CString query_str = "delete from gnutella_hash.hash_table";
	ret = mysql_query(p_raw_data_conn, query_str);
	delete_records_from_hash_table = (UINT)mysql_affected_rows(p_raw_data_conn);
	
	query_str = "insert ignore into gnutella_hash.hash_table(sha1,filesize) select sha1,filesize from gnutella_hash.temp_hash_table";
	ret = mysql_query(p_raw_data_conn, query_str);
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " UpdateHashTable(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret == -1)
		return 0;
	UINT inserted_records = (UINT)mysql_affected_rows(p_raw_data_conn);
	return inserted_records;
}

//
//
//
int ProcessDBInterface::DeleteTable(const char* table)
{
	int ret;

	CString query_str = "drop table ";
	query_str += table;

	ret = mysql_query(p_raw_data_conn,query_str);
	
	if(mysql_error(p_raw_data_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " DeleteTable(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_raw_data_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	return ret;
}

//
//
//
bool ProcessDBInterface::ProcessRealTimeSupply(ProjectKeywords& project)
{
	CString project_name = project.m_project_name.c_str();
	project_name.Replace('\\','_');			// replace the backslash with _
	project_name.Replace('\'', '_');		// replace the single quote "'" with _
	project_name.Replace(' ', '_');
	project_name.Replace('-', '_');
	project_name.Replace('&', '_');
	project_name.Replace('!', '_');
	project_name.Replace('$', '_');
	project_name.Replace('@', '_');
	project_name.Replace('%', '_');
	project_name.Replace('(', '_');
	project_name.Replace(')', '_');
	project_name.Replace('+', '_');
	project_name.Replace('~', '_');
	project_name.Replace('*', '_');
	project_name.Replace('.', '_');
	project_name.Replace(',', '_');
	project_name.Replace('?', '_');
	project_name.Replace(':', '_');
	project_name.Replace(';', '_');
	project_name.Replace('"', '_');
	project_name.Replace('/', '_');
	project_name.Replace('#', '_');

	//truncate the project name if its length is greater than 50
	if(project_name.GetLength() > 50)
	{
		project_name.Truncate(50);
	}

	CString table = "SUPPLY_TABLE_";
	table += project_name;
	if(project.m_supply_keywords.v_keywords.size()) //multiple tracks projects/audio project
	{
		for(UINT i=0;i<project.m_supply_keywords.v_keywords.size();i++)
		{
			UINT num_row=0;
			CString query="select ip,sha1 from ";
			query+=table;
			query+=" where track = ";
			query.AppendFormat("%d",project.m_supply_keywords.v_keywords[i].m_track);
	
			int ret = mysql_query(p_raw_data_conn, query);
			if (ret != 0)
			{ // check for error
				CStdioFile file;
				if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
				{
					file.SeekToEnd();
					CString log;
					log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
					log += " ProcessRealTimeSupply(...) ";
					log += query;
					log += "\n";
					log += "Error: ";
					log += mysql_error(p_raw_data_conn);
					log += "\n\n";
					file.WriteString(log);
					file.Close();
				}

				TRACE(mysql_error(p_raw_data_conn));
				TRACE("\n");
				TRACE(query);
				TRACE("\n");
				string error_message = mysql_error(p_raw_data_conn);
				if(error_message.find("Lost connection to MySQL server during query") == 0 || 
					error_message.find("MySQL server has gone away") == 0)
				{
					if(OpenRawDataConnection())
					{
						return ProcessRealTimeSupply(project);
					}
				}
				else
					return false;
			}

			MYSQL_RES *res=NULL;
			MYSQL_ROW row;
			res=mysql_store_result(p_raw_data_conn);	// allocates memory
			vector<int>IPs;
			vector<string>sha1s;
			if(res != NULL)
			{
				num_row = (UINT)mysql_num_rows(res);
				if(num_row > 0)
				{
					while ((row = mysql_fetch_row(res)))
					{
						if(row[0]!=NULL && row[1]!=NULL)
						{
							int ip=0;
							string sha1;
							sscanf(row[0],"%d", &ip);
							IPs.push_back(ip);
							sha1=row[1];
							sha1s.push_back(sha1);
						}
					}
				}
			}
			mysql_free_result(res);	// free memory
			if(num_row > 0)
			{
				if(IPs.size()==sha1s.size())
				{
					if(strlen(project.m_artist_name.c_str())>0)
					{
						InsertSupplyIPResultsToProcessedDB(IPs,sha1s,project.m_artist_name.c_str(),
							project.m_album_name.c_str(),
							project.m_supply_keywords.v_keywords[i].m_track_name.c_str());
					}
					else
					{
						InsertSupplyIPResultsToProcessedDB(IPs,sha1s,project.m_project_name.c_str(),
							project.m_album_name.c_str(),
							project.m_supply_keywords.v_keywords[i].m_track_name.c_str());
					}
				}
				else
				{
					AfxMessageBox("IPs.size() != sha1s.size()");
				}
			}
		}
		return true;
	}
	else
	{
		UINT num_row=0;
		CString query="select ip,sha1 from ";
		query+=table;
		query+=" where track = 0";

		int ret = mysql_query(p_raw_data_conn, query);
		if (ret != 0)
		{ // check for error
			CStdioFile file;
			if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();
				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += " ProcessRealTimeSupply(...) ";
				log += query;
				log += "\n";
				log += "Error: ";
				log += mysql_error(p_raw_data_conn);
				log += "\n\n";
				file.WriteString(log);
				file.Close();
			}

			TRACE(mysql_error(p_raw_data_conn));
			TRACE("\n");
			TRACE(query);
			TRACE("\n");
			string error_message = mysql_error(p_raw_data_conn);
			if(error_message.find("Lost connection to MySQL server during query") == 0 || 
				error_message.find("MySQL server has gone away") == 0)
			{
				if(OpenRawDataConnection())
				{
					return ProcessRealTimeSupply(project);
				}
			}
			else
				return false;
		}

		MYSQL_RES *res=NULL;
		MYSQL_ROW row;
		res=mysql_store_result(p_raw_data_conn);	// allocates memory
		vector<int>IPs;
		vector<string>sha1s;
		if(res != NULL)
		{
			num_row = (UINT)mysql_num_rows(res);
			if(num_row > 0)
			{
				while ((row = mysql_fetch_row(res)))
				{
					if(row[0]!=NULL && row[1]!=NULL)
					{
						int ip=0;
						string sha1;
						sscanf(row[0],"%d", &ip);
						IPs.push_back(ip);
						sha1=row[1];
						sha1s.push_back(sha1);
					}
				}
			}
		}
		mysql_free_result(res);	// free memory
		if(num_row > 0)
		{
			if(IPs.size()==sha1s.size())
			{
				if(strlen(project.m_artist_name.c_str())>0)
					InsertSupplyIPResultsToProcessedDB(IPs,sha1s,project.m_artist_name.c_str());
				else
					InsertSupplyIPResultsToProcessedDB(IPs,sha1s,project.m_project_name.c_str());
			}
			else
			{
				AfxMessageBox("IPs.size() != sha1s.size()");
			}
		}
		return true;
	}
}

//
//
//
bool ProcessDBInterface::InsertSupplyIPResultsToProcessedDB(vector<int>&IPs, vector<string>& sha1s,const char* artist_name, const char* album_name, const char* track_name)
{
	char temp[32];
	CString table=artist_name;
	table.MakeLower();
	table.Replace('\\','_');			// replace the backslash with _
	table.Replace('\'', '_');		// replace the single quote "'" with _
	table.Replace(' ', '_');
	table.Replace('-', '_');
	table.Replace('&', '_');
	table.Replace('!', '_');
	table.Replace('$', '_');
	table.Replace('@', '_');
	table.Replace('%', '_');
	table.Replace('(', '_');
	table.Replace(')', '_');
	table.Replace('+', '_');
	table.Replace('~', '_');
	table.Replace('*', '_');
	table.Replace('.', '_');
	table.Replace(',', '_');
	table.Replace('?', '_');
	table.Replace(':', '_');
	table.Replace(';', '_');
	table.Replace('"', '_');
	table.Replace('/', '_');
	table.Replace('#', '_');

	//truncate the project name if its length is greater than 50
	if(table.GetLength() > 50)
	{
		table.Truncate(50);
	}
	CString new_table_name;
	CreateDBTable(table,new_table_name);
	CString query = "INSERT IGNORE INTO ";
	query += new_table_name;
	query += " (date,album, title, ip, hash) VALUES ";

	CString album = album_name;
	album.Replace("\\", "\\\\");
	album.Replace("'", "''");

	CString artist = artist_name;
	artist.Replace("\\", "\\\\");
	artist.Replace("'", "''");

	CString track=track_name;
	track.Replace("\\", "\\\\");
	track.Replace("'", "''");

	for(UINT i=0; i<IPs.size(); i++)
	{
		query += "(now(),'";
		query += album;
		query += "','";
		query += track;
		query += "',";
		query += itoa(IPs[i],temp,10);
		query += ",'";
		query += sha1s[i].c_str();
		query += "')";
		if(i != IPs.size()-1)
		{
			query += ",";
		}
	}
	int ret = mysql_query(p_supply_conn,query);
	if (ret != 0)
	{ // check for error
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertSupplyIPResultsToProcessedDB(...) ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";
			file.WriteString(log);
			file.Close();
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");

		string error_message = mysql_error(p_supply_conn);
		if(error_message.find("Lost connection to MySQL server during query") == 0 || 
			error_message.find("MySQL server has gone away") == 0)
		{
			if(OpenSupplyConnection())
			{
				return InsertSupplyIPResultsToProcessedDB(IPs, sha1s, artist_name,album_name,track_name);
			}
		}
	}
	return InsertSupplyResultsToProcessedDB(artist,album,track,(UINT)IPs.size());
}

//
//
//
bool ProcessDBInterface::InsertSupplyResultsToProcessedDB(const char* artist, const char* album, const char* track, UINT value)
{
	CString query = "insert into midata.gnutella_supply (artist, album, title, value) VALUES ('";
	query += artist;
	query += "','";
	query += album;
	query += "','";
	query += track;
	query += "',";
	query.AppendFormat("%u)",value);
	int ret = mysql_query(p_supply_conn,query);
	if (ret != 0)
	{ // check for error
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertSupplyResultsToProcessedDB(...) ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";
			file.WriteString(log);
			file.Close();
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");
		string error_message = mysql_error(p_supply_conn);
		if(error_message.find("Lost connection to MySQL server during query") == 0 || 
			error_message.find("MySQL server has gone away") == 0)
		{
			if(OpenSupplyConnection())
			{
				return InsertSupplyResultsToProcessedDB(artist,album,track,value);
			}
		}
		else
			return false;
	}
	return true;
}

//
//
//
bool ProcessDBInterface::DeleteAllRawData(const char* project_name)
{
	CString project=project_name;
	project.Replace('\\','_');			// replace the backslash with _
	project.Replace('\'', '_');		// replace the single quote "'" with _
	project.Replace(' ', '_');
	project.Replace('-', '_');
	project.Replace('&', '_');
	project.Replace('!', '_');
	project.Replace('$', '_');
	project.Replace('@', '_');
	project.Replace('%', '_');
	project.Replace('(', '_');
	project.Replace(')', '_');
	project.Replace('+', '_');
	project.Replace('~', '_');
	project.Replace('*', '_');
	project.Replace('.', '_');
	project.Replace(',', '_');
	project.Replace('?', '_');
	project.Replace(':', '_');
	project.Replace(';', '_');
	project.Replace('"', '_');
	project.Replace('/', '_');
	project.Replace('#', '_');

	//truncate the project name if its length is greater than 50
	if(project.GetLength() > 50)
	{
		project.Truncate(50);
	}

	CString table = "SUPPLY_TABLE_";
	table += project;

	CString query = "delete from ";
	query += table;

	int ret = mysql_query(p_raw_data_conn,query);
	if (ret != 0)
	{ // check for error
		string error_message = mysql_error(p_raw_data_conn);
		if(error_message.find("Lost connection to MySQL server during query") == 0 || 
			error_message.find("MySQL server has gone away") == 0)
		{
			if(OpenRawDataConnection())
			{
				return DeleteAllRawData(project_name);
			}
		}
		else
		{
			CStdioFile file;
			if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();
				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += " DeleteAllRawData(...) ";
				log += query;
				log += "\n";
				log += "Error: ";
				log += mysql_error(p_raw_data_conn);
				log += "\n\n";
				file.WriteString(log);
				file.Close();
			}

			TRACE(mysql_error(p_raw_data_conn));
			TRACE("\n");
			TRACE(query);
			TRACE("\n");
			return false;
		}
	}
	return true;
}

//
//
//
bool ProcessDBInterface::CreateDBTable(const char* table, CString& new_table_name)
{
	char* table_name = new char[strlen(table)+1];
	strcpy(table_name,table);
	bool all_digit=true;
	for(int i=0;i<(int)strlen(table);i++)
	{
		if(!isdigit(table_name[i]))
		{
			all_digit=false;
			break;
		}
	}
	if(all_digit)
	{
		new_table_name = "a";
		new_table_name+=table;
	}
	else
		new_table_name=table;
	delete [] table_name;
	if(new_table_name.Compare("case")==0)
		new_table_name="acase";

	CString query = "create table if not exists ";
	query += new_table_name;
	query += " (date date, album varchar(255),title varchar(255),ip int,hash varchar(32),primary key (date,ip,hash),index(date),index(album),index(title))";

	int ret = mysql_query(p_supply_conn,query);
	if (ret != 0)
	{ // check for error
		string error_message = mysql_error(p_supply_conn);
		if(error_message.find("Lost connection to MySQL server during query") == 0 || 
			error_message.find("MySQL server has gone away") == 0)
		{
			if(OpenSupplyConnection())
			{
				return CreateDBTable(table,new_table_name);
			}
		}
		else
		{
			CStdioFile file;
			if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();
				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += " CreateDBTable(...) ";
				log += query;
				log += "\n";
				log += "Error: ";
				log += mysql_error(p_supply_conn);
				log += "\n\n";
				file.WriteString(log);
				file.Close();
			}

			TRACE(mysql_error(p_supply_conn));
			TRACE("\n");
			TRACE(query);
			TRACE("\n");
			return false;
		}
	}
	return true;
}

//
//
//
int ProcessDBInterface::DeleteProcessedIPs(const char* table)
{
	CString query = "delete from ";
	query += table;
	query += " where to_days(now()) - to_days(date) > 7";
	int ret = mysql_query(p_supply_conn,query);
	if (ret != 0)
	{ // check for error
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " DeleteProcessedIPs(...) ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";
			file.WriteString(log);
			file.Close();
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");

		string error_message = mysql_error(p_supply_conn);
		if(error_message.find("Lost connection to MySQL server during query") == 0 || 
			error_message.find("MySQL server has gone away") == 0)
		{
			if(OpenSupplyConnection())
			{
				return DeleteProcessedIPs(table);
			}
		}
	}
	return ret;
}

//
//
//
int ProcessDBInterface::OptimizeProcessedIPs(ProjectKeywords& project)
{
	CString table=project.m_project_name.c_str();
	table.MakeLower();
	table.Replace('\\','_');			// replace the backslash with _
	table.Replace('\'', '_');		// replace the single quote "'" with _
	table.Replace(' ', '_');
	table.Replace('-', '_');
	table.Replace('&', '_');
	table.Replace('!', '_');
	table.Replace('$', '_');
	table.Replace('@', '_');
	table.Replace('%', '_');
	table.Replace('(', '_');
	table.Replace(')', '_');
	table.Replace('+', '_');
	table.Replace('~', '_');
	table.Replace('*', '_');
	table.Replace('.', '_');
	table.Replace(',', '_');
	table.Replace('?', '_');
	table.Replace(':', '_');
	table.Replace(';', '_');
	table.Replace('"', '_');
	table.Replace('/', '_');
	table.Replace('#', '_');

	//truncate the project name if its length is greater than 50
	if(table.GetLength() > 50)
	{
		table.Truncate(50);
	}
	CString new_table_name;
	CreateDBTable(table,new_table_name);
	CString query = "OPTIMIZE TABLE ";
	query += new_table_name;
	int ret = mysql_query(p_supply_conn,query);
	if (ret != 0)
	{ // check for error
		CStdioFile file;
		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " OptimizeProcessedIPs(...) ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";
			file.WriteString(log);
			file.Close();
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");

		string error_message = mysql_error(p_supply_conn);
		if(error_message.find("Lost connection to MySQL server during query") == 0 || 
			error_message.find("MySQL server has gone away") == 0)
		{
			if(OpenSupplyConnection())
			{
				return OptimizeProcessedIPs(project);
			}
		}
	}
	return ret;
}
