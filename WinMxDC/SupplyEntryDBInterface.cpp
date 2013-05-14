#include "StdAfx.h"
#include "supplyentrydbinterface.h"
#include "HashConversion.h"
#define WM_SUPPLY_MANAGER_THREAD_PROJECT	WM_USER+18

SupplyEntryDBInterface::SupplyEntryDBInterface(void)
{
}

SupplyEntryDBInterface::~SupplyEntryDBInterface(void)
{
}

//
//
//
bool SupplyEntryDBInterface::RetrieveSupplyEntry(ProjectKeywords& keyword, vector<QueryHit>& supplies, HWND hwnd)
{
	int ret = -1;
	
	CString project = keyword.m_project_name.c_str();

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


	char temp[32];
	CString query_str;
	int track;

	for(int i=0; i==0 || i<keyword.m_supply_keywords.v_keywords.size(); i++)
	{
		if(keyword.m_supply_keywords.v_keywords.size() > 0)
			track = keyword.m_supply_keywords.v_keywords[i].m_track;
		else
			 track = 0;
		
		//post the query status to the main window
		char status[256+1];
		sprintf(status, "%s - Track: %d", keyword.m_project_name.c_str(), track);
		PostMessage(hwnd,WM_SUPPLY_MANAGER_THREAD_PROJECT,(WPARAM)status,0);

		query_str = "SELECT distinct hash, filename, filesize, bitrate, frequency, duration, count(*) as total from ";
		query_str += table;
		query_str += " where track = ";
		query_str += itoa(track, temp, 10);
		if(keyword.m_search_type == ProjectKeywords::audio)
			query_str += " and filesize <= 10000000 ";
		query_str += " group by hash";
		query_str += " order by total desc limit ";

		//if this is single, doubling up the limit
		if(track!=0)
		{
			if(keyword.m_supply_keywords.v_keywords[i].m_single == true)
				query_str += itoa(400, temp, 10);
			else
				//query_str += itoa(200, temp, 10);
				continue; //not protecting non-singles
		}
		else
			query_str += itoa(200, temp, 10);

		ret = mysql_query(p_supply_conn, query_str);

		if(mysql_error(p_supply_conn)[0] != '\0')
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
				log += mysql_error(p_supply_conn);
				log += "\n\n";

				file.WriteString(log);
				file.Close();
			}
			else
			{
				DWORD error = GetLastError();
				//MessageBox(NULL, "File Open Error: SQL_Supply_Entry_Error_Log.txt", "File Open Error", MB_OK);
			}

			TRACE(mysql_error(p_supply_conn));
			TRACE("\n");
			TRACE(query_str);
			TRACE("\n");

			//retry this query and reconnect to db
			mysql_ping(p_supply_conn);

			i--;
			if(i<0)
				i=0;
			continue;
		}

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
					QueryHit info;
					if(row[0]!=NULL)
					{
						HashConversion fthash;
						fthash.HashDecode16(row[0]);
						memset(&info.m_hash,0,sizeof(info.m_hash));
						memcpy(info.m_hash,fthash.m_data,sizeof(info.m_hash));
					}
					if(row[1]!=NULL)
						info.m_filename = row[1];
					if(row[2]!=NULL)
						info.m_file_size = atol(row[2]);
					if(row[3]!=NULL)
						info.m_bitrate = atoi(row[3]);
					if(row[4]!=NULL)
						info.m_sample_frequency = atoi(row[4]);
					if(row[5]!=NULL)
						info.m_duration = atoi(row[5]);


					info.m_track = track;
					info.m_project_id = keyword.m_id;
					info.m_project = keyword.m_project_name.c_str();
					
					switch(keyword.m_search_type)
					{
						case ProjectKeywords::search_type::audio:
						{
							info.m_media_type = MEDIA_TYPE_AUDIO;
							break;
						}
						case ProjectKeywords::search_type::video:
						{
							info.m_media_type = MEDIA_TYPE_VIDEO;
							break;
						}
						case ProjectKeywords::search_type::software:
						{
							info.m_media_type = MEDIA_TYPE_SOFTWARE;
							break;
						}
					}
					/*
					int popularity=0; //share more popular title
					if(row[3]!=NULL)
						popularity=atoi(row[3]);
					popularity /= 100;
					if(popularity > 100)
						popularity = 100;
					if(popularity <= 0)
						popularity = 1;
					for(int j=0;j<popularity;j++)
						supplies.push_back(info);
					*/
					supplies.push_back(info);
				}
			}
		}
		mysql_free_result(res);	// free memory

	}

	if(ret==0)
		return true;
	else
		return false;
}
