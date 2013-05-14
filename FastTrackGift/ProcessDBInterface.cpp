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

//
//
//
int ProcessDBInterface::GetLastProcessedDateForSupply(CString project, char* date)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "SELECT MAX(date) from fasttrack_supply where project='";
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
bool ProcessDBInterface::ProcessSupply(CString project, char* start, char* end, char* on_date, int total_tracks)
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
	char temp[16];

	CString query_str = "SELECT distinct search_id from ";
	query_str += table;
	query_str += " where timestamp >= '";
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
		return inserted_result;

	vector<CString> search_ids;
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
					search_ids.push_back(row[0]);
			}
		}
	}
	mysql_free_result(res);	// free memory

	vector<TrackSupply> track_supplies;
	//vector<TrackSupply> fake_track_supplies;

	UINT i,j;
	for(i=0; i<search_ids.size();i++)
	{
		for(int track=0; track<=total_tracks; track++)
		{
			query_str = "select SQL_CALC_FOUND_ROWS count(*) from ";
			query_str += table;
			query_str += " where search_id = ";
			query_str += search_ids[i];
			query_str += " and track=";
			query_str += itoa(track,temp,10);
			//query_str += " and from_md='N' ";
			query_str += " group by hash,ip limit 1";
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
				return inserted_result;
			res=mysql_store_result(p_raw_data_conn);	// allocates memory
			if(res != NULL)
			{
				UINT num_row = (UINT)mysql_num_rows(res);
				if(num_row > 0)
				{
					row = mysql_fetch_row(res);
				}
			}
			mysql_free_result(res);	// free memory

			query_str = "select found_rows()";
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
				return inserted_result;
			res=mysql_store_result(p_raw_data_conn);	// allocates memory
			if(res != NULL)
			{
				UINT num_row = (UINT)mysql_num_rows(res);
				if(num_row > 0)
				{
					while ((row = mysql_fetch_row(res)))
					{
						if(row[0]!=NULL)
						{
							TrackSupply ts;
							ts.m_track = track;
							sscanf(row[0],"%u", &ts.m_supply);
							track_supplies.push_back(ts);
						}
					}
				}
			}
			mysql_free_result(res);	// free memory
		}

/*
		//getting fake supply
		query_str = "SELECT track, count(*) from ";
		query_str += table;
		query_str += " where search_id = ";
		query_str += search_ids[i];
		query_str += " and from_md='Y' ";
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

		//MYSQL_RES *res=NULL;
		//MYSQL_ROW row;

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
						fake_track_supplies.push_back(ts);
					}
				}
			}
		}
		mysql_free_result(res);	// free memory

		for(track=0; track<=total_tracks; track++)
		{
			query_str = "select SQL_CALC_FOUND_ROWS count(*) from ";
			query_str += table;
			query_str += " where search_id = ";
			query_str += search_ids[i];
			query_str += " and track=";
			query_str += itoa(track,temp,10);
			query_str += " and from_md='Y' ";
			query_str += " group by hash,ip limit 1";
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
				return inserted_result;

			res=mysql_store_result(p_raw_data_conn);	// allocates memory
			if(res != NULL)
			{
				UINT num_row = (UINT)mysql_num_rows(res);
				if(num_row > 0)
				{
					row = mysql_fetch_row(res);
				}
			}
			mysql_free_result(res);	// free memory

			query_str = "select found_rows()";
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
				return inserted_result;
			res=mysql_store_result(p_raw_data_conn);	// allocates memory
			if(res != NULL)
			{
				UINT num_row = (UINT)mysql_num_rows(res);
				if(num_row > 0)
				{
					while ((row = mysql_fetch_row(res)))
					{
						if(row[0]!=NULL)
						{
							TrackSupply ts;
							ts.m_track = track;
							sscanf(row[0],"%u", &ts.m_supply);
							fake_track_supplies.push_back(ts);
						}
					}
				}
			}
			mysql_free_result(res);	// free memory
		}*/
	} //for

	//adding fake supply
//	query_str = "SELECT track,SUM(supply) from ";
//	query_str += "fasttrack_fake_supply.";
//	query_str += table;
//	query_str += " group by track";
//	ret = mysql_query(p_raw_data_conn, query_str);
//	if(mysql_error(p_raw_data_conn)[0] != '\0')
//	{
///*
//		CStdioFile file;
//		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
//		{
//			file.SeekToEnd();
//
//			CString log;
//			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
//			log += " ProcessSupply(...) ";
//			log += query_str;
//			log += "\n";
//			log += "Error: ";
//			log += mysql_error(p_raw_data_conn);
//			log += "\n\n";
//
//			file.WriteString(log);
//			file.Close();
//		}
//		else
//		{
//			DWORD error = GetLastError();
//			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
//		}
//*/
//		TRACE(mysql_error(p_raw_data_conn));
//		TRACE("\n");
//		TRACE(query_str);
//		TRACE("\n");
//
//	}
//	else
//	{
//		res=mysql_store_result(p_raw_data_conn);	// allocates memory
//		if(res != NULL)
//		{
//			UINT num_row = (UINT)mysql_num_rows(res);
//			if(num_row > 0)
//			{
//				while ((row = mysql_fetch_row(res)))
//				{
//					if(row[0]!=NULL && row[1]!=NULL)
//					{
//						TrackSupply ts;
//						sscanf(row[0],"%u", &ts.m_track);
//						sscanf(row[1],"%u", &ts.m_supply);
//						track_supplies.push_back(ts);
//					}
//				}
//			}
//		}
//		mysql_free_result(res);	// free memory
//	}

	//calculate average supply
	vector<TrackSupply> average_supplies;
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
		if(search_ids.size() > 1)
			average_supplies[i].m_supply /= (int)search_ids.size();
		char track[32];
		char supply[32];
		sprintf(track, "%d", average_supplies[i].m_track);
		sprintf(supply, "%u", average_supplies[i].m_supply);
		InsertProcessedSupplyData(project, on_date, track, supply);
		inserted_result=true;
	}
/*
	//calculate fake average supply
	vector<TrackSupply> fake_average_supplies;
	for(i=0; i<fake_track_supplies.size();i++)
	{
		bool found = false;
		for(j=0; j<fake_average_supplies.size();j++)
		{
			if(fake_track_supplies[i].m_track == fake_average_supplies[j].m_track)
			{
				found = true;
				break;
			}
		}
		if(!found)
			fake_average_supplies.push_back(fake_track_supplies[i]);
		else
			fake_average_supplies[j].m_supply += fake_track_supplies[i].m_supply;
	}

	for(i=0; i<fake_average_supplies.size();i++)
	{
		fake_average_supplies[i].m_supply /= (int)search_ids.size();
		char track[32];
		char supply[32];
		sprintf(track, "%d", fake_average_supplies[i].m_track);
		sprintf(supply, "%u", fake_average_supplies[i].m_supply);
		InsertProcessedFakeSupplyData(project, on_date, track, supply);
		inserted_result=true;
	}
*/
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

	CString query_str = "INSERT INTO fasttrack_supply";
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
int ProcessDBInterface::InsertProcessedFakeSupplyData(CString project, char* date, const char* track, const char* supply)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query_str = "INSERT INTO fasttrack_fake_supply";
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
			log += " InsertProcessedFakeSupplyData(...) ";
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
	query_str += "'";// or from_md='Y'";

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

	//delete fake supply table
//	query_str="DELETE QUICK FROM ";
//	query_str += "fasttrack_fake_supply.";
//	query_str += table;
//	query_str += " where timestamp < '";
//	query_str += timestamp;
//	query_str += "'";
//	ret = mysql_query(p_raw_data_conn,query_str);
//	if(mysql_error(p_raw_data_conn)[0] != '\0')
//	{
///*		CStdioFile file;
//		if( file.Open("SQL_Process_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
//		{
//			file.SeekToEnd();
//
//			CString log;
//			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
//			log += " DeleteRawData(...fake supply tabke) ";
//			log += query_str;
//			log += "\n";
//			log += "Error: ";
//			log += mysql_error(p_raw_data_conn);
//			log += "\n\n";
//
//			file.WriteString(log);
//			file.Close();
//		}
//		else
//		{
//			DWORD error = GetLastError();
//			MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
//		}
//*/
//		TRACE(mysql_error(p_raw_data_conn));
//		TRACE("\n");
//		TRACE(query_str);
//		TRACE("\n");
//	}
	return delete_records;
}

//
//
//
int ProcessDBInterface::OptimizeTables(vector<CString>& tables)
{
	UINT i;
	//vector<CString> new_tables = tables;
	//for(i=0; i <tables.size();i++)
	//{
	//	CString fake_table = "fasttrack_fake_supply.";
	//	fake_table += tables[i];
	//	new_tables.push_back(tables[i]);
	//	new_tables.push_back(fake_table);
	//}
	int ret;

	CString query_str = "OPTIMIZE TABLE ";
	for(i=0; i < tables.size(); i++)
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
int	ProcessDBInterface::GetAllSupplyTables(vector<CString>& tables)
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
					if(strcmp((const char*)row[0], "gnutella_guid_index")!= 0)
						tables.push_back( (const char*)row[0] );
				}
			}
		}
	}

	mysql_free_result(res);	// free memory
	return ret;
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