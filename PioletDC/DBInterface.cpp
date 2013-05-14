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
int DBInterface::InsertRawPioletDemand(char* table, CString project, int track, CString query, CString& timestamp, UINT ip)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	query.Replace("\\","_");

	project.Replace("\'", "\\\'");		// replace the single quote "'"
	query.Replace("\'", "\\\'");

	int ret;
	char temp[32];
	memset(temp, 0, sizeof(temp));

	CString query_str = "INSERT INTO ";
	query_str += table;
	query_str += " (project,track,query,timestamp,ip) VALUES ('";

	query_str += project;
	query_str += "',";
	query_str += itoa(track, temp, 10);
	query_str += ",'";
	query_str += query;
	query_str += "','";
	query_str += timestamp;
	query_str += "',";
	query_str += ultoa(ip,temp,10);
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
			log += " InsertRawPioletDemand(...) ";
			log += query;
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
		TRACE(query);
		TRACE("\n");
	}
	return ret;
}

//
//
//
int DBInterface::InsertRawPioletSupply(char* table,
							CString& project,
							UINT ip,
							CString& nickname,
							UINT file_size,
							int track,
							int bitrate,
							int sampling_rate,
							int stereo,
							int song_length,
							CString& filename,
							char* md5,
							CString& timestamp,
							UINT header_counter)
{
	int ret=0;

	project.Replace("\\","\\\\");			// replace the backslash
	filename.Replace("\\","\\\\");
	nickname.Replace("\\","\\\\");

	project.Replace("\'", "\\\'");		// replace the single quote "'"
	filename.Replace("\'", "\\\'");
	nickname.Replace("\'", "\\\'");

	char temp[32];
	memset(temp, 0, sizeof(temp));

	CString query = "INSERT INTO ";
	query += table;
	query += " (project,ip,nickname,file_size,track,bitrate,sampling_rate,stereo,song_length,filename,md5,timestamp,header_counter) VALUES ('";

	query += project;
	query += "',";
	query += ultoa(ip,temp,10);
	query += ",'";
	query += nickname;
	query += "',";
	query += ultoa(file_size, temp, 10);
	query += ",";
	query += itoa(track,temp,10);
	query += ",";
	query += itoa(bitrate,temp,10);
	query += ",";
	query += itoa(sampling_rate,temp,10);
	query += ",";
	query += itoa(stereo,temp,10);
	query += ",";
	query += itoa(song_length,temp,10);
	query += ",'";
	query += filename;
	query += "','";
	query += md5;
	query += "','";
	query += timestamp;
	query += "',";
	query += ultoa(header_counter, temp, 10);
	query += ")";

	ret = mysql_query(p_conn,query);

	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertRawPioletSupply(...) ";
			log += query;
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
		TRACE(query);
		TRACE("\n");
	}

	return ret;
}

/*
//
//
//
int DBInterface::InsertGUID(char* table, GUID& guid, CString& project, const char* timestamp)
{
	project.Replace("\\","_");			// replace the backslash with a "_"
	project.Replace("\'", "\\\'");		// replace the single quote "'"

	int ret;

	CString query = "INSERT INTO ";
	query += table;
	query += " (project,timestamp,guid) VALUES ('";

	query += project;
	query += "','";
	query += timestamp;
	query += "','";

	char escape_string[sizeof(GUID)*2+1];
	memset(&escape_string,0,sizeof(escape_string));
	UINT guid_size = mysql_real_escape_string(p_conn, escape_string, (const char*)&guid, sizeof(GUID));

	UINT query_length = (UINT)query.GetLength() + guid_size + strlen("')")+1;
	
	char* temp_buf = new char[query.GetLength()+1];
	memset(temp_buf,0,query.GetLength()+1);
	strcpy(temp_buf,query);
	
	byte* query_buf = new byte[query_length];
	memset(query_buf, 0, query_length);
	memcpy(query_buf, temp_buf, query.GetLength());

	memcpy(&query_buf[query.GetLength()], &escape_string, guid_size);
	memcpy(&query_buf[query.GetLength()+guid_size], "')", 2);
	query_buf[query_length-1] = '\0';

	ret = mysql_real_query(p_conn, (const char*)query_buf, query_length);
	delete temp_buf;
	delete query_buf;

	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertGUID(...) ";
			log += query;
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
		TRACE(query);
		TRACE("\n");
	}
	return ret;
}

//
//
//
UINT DBInterface::GetGUIDIndex(GUID& guid)
{
	int ret =0;
	UINT guid_index=0;

	CString query = "SELECT guid_index from ";
	query += GNUTELLA_GUID_INDEX_TALBE;
	query += " where guid='";

	char escape_string[sizeof(GUID)*2+1];
	memset(&escape_string,0,sizeof(escape_string));
	UINT guid_size = mysql_real_escape_string(p_conn, escape_string, (const char*)&guid, sizeof(GUID));

	UINT query_length = (UINT)query.GetLength() + guid_size + strlen("'")+1;
	
	char* temp_buf = new char[query.GetLength()+1];
	memset(temp_buf,0,query.GetLength()+1);
	strcpy(temp_buf,query);
	
	byte* query_buf = new byte[query_length];
	memset(query_buf, 0, query_length);
	memcpy(query_buf, temp_buf, query.GetLength());

	memcpy(&query_buf[query.GetLength()], &escape_string, guid_size);
	query_buf[query_length-2] = '\'';
	query_buf[query_length-1] = '\0';

	ret = mysql_real_query(p_conn, (const char*)query_buf, query_length);
	delete temp_buf;
	delete query_buf;

	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetGUIDIndex(...) ";
			log += query;
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
		TRACE(query);
		TRACE("\n");
		return 0;
	}
	else
	{
		MYSQL_RES *res_set; 
		MYSQL_ROW row; 
		unsigned int i; 

		res_set = mysql_store_result(p_conn);
		if(res_set != NULL)
		{
			unsigned int numrows = (unsigned int)mysql_num_rows(res_set);

			while ((row = mysql_fetch_row(res_set)) != NULL) 
			{ 
				for (i=0; i<mysql_num_fields(res_set); i++)
				{
					sscanf((char*)row[i], "%u", &guid_index);
				}
			}
			mysql_free_result(res_set);	// free memory
		}
	}
	return guid_index;
}
*/