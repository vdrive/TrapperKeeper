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

	CString query_str = "SELECT MAX(date) from piolet_demand where project='";
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
			log += " GetLastProcessedDateForDemand(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
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
int ProcessDBInterface::GetLastProcessedDateForSupply(CString project, char* date)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT MAX(date) from piolet_supply where project='";
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
			log += " GetLastProcessedDateForSupply(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
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
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT MIN(timestamp) from piolet_raw_demand where project='";
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
int ProcessDBInterface::GetFirstSupplyDataInsertionTimestamp(CString project, char* timestamp)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT MIN(timestamp) from piolet_raw_supply where project='";
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
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT track, count(*), count(distinct(ip)) from piolet_raw_demand where project='";
	query_str += project;
	query_str += "' and timestamp >= '";
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
				if(row[0] != NULL && row[1] != NULL && row[2] != NULL)
				{
					InsertProcessedDemandData(project, on_date, row[0], row[1], row[2]);
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
int ProcessDBInterface::InsertProcessedDemandData(CString project, char* date, const char* track, const char* demand, const char* unique_user)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "INSERT INTO piolet_demand";
	query_str += " (date,project,track,demand,demand_from_unique_user) VALUES ('";
	query_str += date;
	query_str += "','";
	query_str += project;
	query_str += "',";
	query_str += track;
	query_str += ",";
	query_str += demand;
	query_str += ",";
	query_str += unique_user;
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
			log += " InsertProcessedDemandData(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
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
bool ProcessDBInterface::ProcessSupply(CString project, char* start, char* end, char* on_date)
{
	bool inserted_result=false;
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT distinct header_counter from piolet_raw_supply where project='";
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

	vector<CString> header_counters;
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
					header_counters.push_back(row[0]);
			}
		}
	}
	mysql_free_result(res);	// free memory

	vector<TrackSupply> track_supplies;
	for(UINT i=0; i<header_counters.size();i++)
	{
		query_str = "SELECT track, count(*) from piolet_raw_supply where header_counter = ";
		query_str += header_counters[i];
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
		average_supplies[i].m_supply /= (int)header_counters.size();
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

	CString query_str = "INSERT INTO piolet_supply";
	query_str += " (date,project,track,supply) VALUES ('";
	query_str += date;
	query_str += "','";
	query_str += project;
	query_str += "',";
	query_str += track;
	query_str += ",";
	query_str += supply;
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
			log += " InsertProcessedSupplyData(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
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
