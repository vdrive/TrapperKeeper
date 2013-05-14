#include "StdAfx.h"
#include "supplyentrydbinterface.h"

SupplyEntryDBInterface::SupplyEntryDBInterface(void)
{
}

SupplyEntryDBInterface::~SupplyEntryDBInterface(void)
{
}

//
//
//
int SupplyEntryDBInterface::RetrieveSupplyEntry(SupplyProject& sp,int entry_limit)
{
	CString project = sp.m_name.c_str();
	project.Replace("\\","\\\\");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	char temp[32];
	CString query_str;
	
	for(int track=0; track<=sp.m_total_tracks; track++)
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
#ifdef _DEBUG
				MessageBox(NULL, "File Open Error: SQL_Supply_Entry_Error_Log.txt", "File Open Error", MB_OK);
#endif
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

//
//
//
int SupplyEntryDBInterface::RetrievePoisonEntry(SupplyProject& sp,int entry_limit)
{
	CString project = sp.m_name.c_str();
	project.Replace("\\","\\\\");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	char temp[32];
	CString query_str;
	
	for(int track=1; track<=sp.m_total_tracks; track++)
	{
		query_str = "SELECT distinct md5, filename, file_size, bitrate,sampling_rate,stereo,song_length, count(*) as total from piolet_raw_supply where project = '";
		query_str += project;
		query_str += "' and track = ";
		query_str += itoa(track, temp, 10);
		query_str += " group by md5";
		query_str += " order by total desc limit ";
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
				log += " RetrievePoisonEntry(...) ";
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
				MessageBox(NULL, "File Open Error: SQL_Supply_Entry_Error_Log.txt", "File Open Error", MB_OK);
#endif
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
						se.MD5(row[0]);
					if(row[1]!=NULL)
						se.Filename(row[1]);
					if(row[2]!=NULL)
						se.m_file_length = atol(row[2]);
					if(row[3]!=NULL)
						se.m_bit_rate = atoi(row[3]);
					if(row[4]!=NULL)
						se.m_sampling_rate = atoi(row[4]);
					if(row[5]!=NULL)
						se.m_stereo = atoi(row[5]);
					if(row[6]!=NULL)
						se.m_song_length = atoi(row[6]);
					if(row[7]!=NULL)
						se.m_popularity = atoi(row[7]);
					se.m_track = track;
					se.Nickname("NULL");
					sp.v_poison_entries.push_back(se);
				}
			}
		}
		mysql_free_result(res);	// free memory
	}

	return ret;
}
