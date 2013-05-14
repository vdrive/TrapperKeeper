#include "StdAfx.h"
#include "supplyentrydbinterface.h"
#define WM_SUPPLY_MANAGER_THREAD_PROJECT						WM_USER+11

SupplyEntryDBInterface::SupplyEntryDBInterface(void)
{
}

SupplyEntryDBInterface::~SupplyEntryDBInterface(void)
{
}

//
//
//
/*
int SupplyEntryDBInterface::RetrieveSupplyEntry(SupplyProject& sp,int entry_limit)
{
	CString project = sp.m_name.c_str();
	project.Replace("\\","\\\\");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	char temp[32];
	CString query_str;
	
	for(int track=1; track<=sp.m_total_tracks; track++)
	{
		query_str = "SELECT nickname,file_size,bitrate,sampling_rate,stereo,song_length,filename from piolet_raw_supply where project = '";
		query_str += project;
		query_str += "' and track = ";
		query_str += itoa(track, temp, 10);
		query_str += " order by rand() limit ";
		query_str += itoa(entry_limit, temp, 10);
		query_str += ";";

		ret = mysql_query(p_conn, query_str);

		if(mysql_error(p_conn)[0] != '\0')
		{
			CStdioFile file;
			if( file.Open("SQL_Supply_Entry_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();

				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += " RetrieveSupplyEntry(...) ";
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
				MessageBox(NULL, "File Open Error: SQL_Supply_Entry_Error_Log.txt", "File Open Error", MB_OK);
			}

			TRACE(mysql_error(p_conn));
			TRACE("\n");
			TRACE(query_str);
			TRACE("\n");
		}

		MYSQL_RES *res=NULL;
		MYSQL_ROW row;

		res=mysql_store_result(p_conn);	// allocates memory
		if(res != NULL)
		{
			UINT num_row = (UINT)mysql_num_rows(res);
			if(num_row > 0)
			{
				while ((row = mysql_fetch_row(res)))
				{
					SupplyEntry se;
					if(row[0]!=NULL)
						se.Nickname(row[0]);
					else
						se.Nickname("NULL");
					if(row[1]!=NULL)
						se.m_file_length = atol(row[1]);
					if(row[2]!=NULL)
						se.m_bit_rate = atoi(row[2]);
					if(row[3]!=NULL)
						se.m_sampling_rate = atoi(row[3]);
					if(row[4]!=NULL)
						se.m_stereo = atoi(row[4]);
					if(row[5]!=NULL)
						se.m_song_length = atoi(row[5]);
					if(row[6]!=NULL)
						se.Filename(row[6]);
					else
						se.Filename("NULL");
					se.m_track = track;
					se.MD5("NULL");
					sp.v_spoof_entries.push_back(se);
				}
			}
		}
		mysql_free_result(res);	// free memory
	}

	return ret;
}
*/
//
//
//
int SupplyEntryDBInterface::RetrieveSupplyEntry(SupplyProject& sp,int entry_limit, HWND hwnd)
{
	CString project = sp.m_name.c_str();
	int index = project.Find("_**Singles**_");
	if(index != -1)
	{
		project.Delete(index,strlen("_**Singles**_"));
	}

//	project.Replace("\\","\\\\");			// replace the backslash with a "_"
//	project.Replace("\'", "\\\'");		// replace the single quote "'"

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

	int ret = -1;

	char temp[32];
	CString query_str;
	
	for(int track=0; track<=sp.m_total_tracks; track++)
	{
		//post the query status to the main window
		char status[256+1];
		sprintf(status, "%s - Track: %d", sp.m_name.c_str(), track);
		PostMessage(hwnd,WM_SUPPLY_MANAGER_THREAD_PROJECT,(WPARAM)status,0);

		query_str = "SELECT distinct sha1, filename, filesize, count(*) as total from ";
		query_str += table;
		query_str += " where track = ";
		query_str += itoa(track, temp, 10);
		query_str += " and spoof='N'";
		query_str += " group by sha1";
		query_str += " order by total desc limit ";
		query_str += itoa(entry_limit, temp, 10);
		//query_str += ";";

		ret = mysql_query(p_conn, query_str);

		if(mysql_error(p_conn)[0] != '\0')
		{
			CStdioFile file;
			if( file.Open("SQL_Supply_Entry_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();

				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += " RetrieveSupplyEntry(...) ";
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
				MessageBox(NULL, "File Open Error: SQL_Supply_Entry_Error_Log.txt", "File Open Error", MB_OK);
			}

			TRACE(mysql_error(p_conn));
			TRACE("\n");
			TRACE(query_str);
			TRACE("\n");
		}

		MYSQL_RES *res=NULL;
		MYSQL_ROW row;

		res=mysql_store_result(p_conn);	// allocates memory
		if(res != NULL)
		{
			UINT num_row = (UINT)mysql_num_rows(res);
			if(num_row > 0)
			{
				while ((row = mysql_fetch_row(res)))
				{
					SupplyEntry se;
					if(row[0]!=NULL)
						se.SHA1(row[0]);
					if(row[1]!=NULL)
						se.Filename(row[1]);
					if(row[2]!=NULL)
						se.m_file_length = atol(row[2]);
					if(row[3]!=NULL)
						se.m_popularity = atoi(row[3]);
					se.m_track = track;
					sp.v_spoof_entries.push_back(se);
				}
			}
		}
		mysql_free_result(res);	// free memory
/*
		//adding the hashes to the hash table for poisoners usage
		query_str = "insert ignore into gnutella_hash.hash_table(sha1,filesize,popularity) select distinct sha1,filesize,count(*) as total from ";
		query_str += table;
		query_str += " where track = ";
		query_str += itoa(track, temp, 10);
		query_str += " and spoof='N'";
		query_str += " group by sha1";
		query_str += " order by total desc limit ";
		query_str += itoa(entry_limit, temp, 10);
		//query_str += ";";
		ret = mysql_query(p_conn, query_str);

		if(mysql_error(p_conn)[0] != '\0')
		{
			CStdioFile file;
			if( file.Open("SQL_Supply_Entry_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();

				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += "Adding Hashes(...) ";
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
				MessageBox(NULL, "File Open Error: SQL_Supply_Entry_Error_Log.txt", "File Open Error", MB_OK);
			}

			TRACE(mysql_error(p_conn));
			TRACE("\n");
			TRACE(query_str);
			TRACE("\n");
		}
*/
	}
	return ret;
}

//
//
//
int SupplyEntryDBInterface::DeleteOldHashes()
{
	CTime current_time = CTime::GetCurrentTime();
	CTime delete_from_date =  current_time - CTimeSpan(2,0,0,0);
	char delete_timestamp[32];
	sprintf(delete_timestamp, "%d%.2d%.2d000000", delete_from_date.GetYear(),delete_from_date.GetMonth(),delete_from_date.GetDay());

	CString query_str = "delete from gnutella_hash.hash_table where timestamp < '";
	query_str += delete_timestamp;
	query_str +="'";
	int ret = mysql_query(p_conn, query_str);

	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Supply_Entry_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " DeleteOldHashes(...) ";
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
			MessageBox(NULL, "File Open Error: SQL_Supply_Entry_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	return ret;
}