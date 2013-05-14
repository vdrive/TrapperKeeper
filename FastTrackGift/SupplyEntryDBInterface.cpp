#include "StdAfx.h"
#include "supplyentrydbinterface.h"
#include "FastTrackMeta.h"
#define WM_FILE_SHARING_MANAGER_THREAD_PROJECT	WM_USER+18

SupplyEntryDBInterface::SupplyEntryDBInterface(void)
{
}

SupplyEntryDBInterface::~SupplyEntryDBInterface(void)
{
}

//
//
//
bool SupplyEntryDBInterface::RetrieveSupplyEntry(ProjectKeywords& keyword, vector<FastTrackFileInfo>& supplies, HWND hwnd)
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
	int retry=0;

	for(int i=0; i==0 || i<keyword.m_supply_keywords.v_keywords.size(); i++)
	{
		if(keyword.m_supply_keywords.v_keywords.size() > 0)
			track = keyword.m_supply_keywords.v_keywords[i].m_track;
		else
			 track = 0;
		
		//post the query status to the main window
		char status[256+1];
		sprintf(status, "%s - Track: %d", keyword.m_project_name.c_str(), track);
		PostMessage(hwnd,WM_FILE_SHARING_MANAGER_THREAD_PROJECT,(WPARAM)status,0);

		query_str = "SELECT distinct hash, filename, filesize, count(*) as total from ";
		query_str += table;
		query_str += " where track = ";
		query_str += itoa(track, temp, 10);
		//query_str += " and from_md='N'";
		query_str += " group by hash";
		query_str += " order by total desc limit ";

		//if this is single, tripling up the limit
		if(track!=0)
		{
			if(keyword.m_supply_keywords.v_keywords[i].m_single == true)
				query_str += itoa(200, temp, 10);
			else
				query_str += itoa(10, temp, 10);
		}
		else
			query_str += itoa(50, temp, 10);

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

			if(retry<=10)
			{
				i--;
				if(i<0)
					i=0;
				retry++;
			}
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
					FastTrackFileInfo info;
					if(row[0]!=NULL)
						info.SetHash(row[0]);
					if(row[1]!=NULL)
						info.m_filename = row[1];
					if(row[2]!=NULL)
						info.m_filesize = atol(row[2]);
					info.m_track_number = track;
					info.m_project_id = keyword.m_id;
					info.m_project_name = keyword.m_project_name.c_str();
					info.m_album = keyword.m_album_name.c_str();
					info.m_artist = keyword.m_artist_name.c_str();
					
					info.m_title = info.m_filename; 
					int del_index = info.m_title.ReverseFind('.');
					if(del_index != -1)
						info.m_title.Delete(del_index,info.m_title.GetLength()-del_index);
					
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

//#ifdef FILE_SERVER_ENABLED
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
/*#else
						supplies.push_back(info);
#endif
*/				}
			}
		}
		mysql_free_result(res);	// free memory

	}


	if(ret==0)
		return true;
	else
		return false;
}
