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
	p_supply_conn=NULL;
	p_demand_conn=NULL;
	p_supply_hash_conn=NULL;
}

DBInterface::~DBInterface()
{

}

//
//
//
bool DBInterface::OpenSupplyConnection(CString ip, CString login, CString pass, CString db)
{
	m_supply_db_address=ip;
	m_supply_db_user=login;
	m_supply_db_password=pass;
	m_supply_db_name=db;

	p_supply_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_supply_conn, ip, login, pass, db, 0, NULL, 0))
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
			log += " DBInterface::OpenSupplyConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_supply_conn);
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
bool DBInterface::OpenSupplyConnection()
{
	CloseSupplyConnection();
	p_supply_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_supply_conn, m_supply_db_address, m_supply_db_user, m_supply_db_password, m_supply_db_name, 0, NULL, 0))
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
			log += " DBInterface::OpenSupplyConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_supply_conn);
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
bool DBInterface::OpenSupplyHashConnection(CString ip, CString login, CString pass, CString db)
{
	p_supply_hash_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_supply_hash_conn, ip, login, pass, db, 0, NULL, 0))
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
			log += " DBInterface::OpenSupplyHashConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_supply_hash_conn);
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
void DBInterface::CloseSupplyConnection()
{
	mysql_close(p_supply_conn);
#ifdef DIFFERENT_HASH_DB
	if(p_supply_hash_conn!=NULL)
		mysql_close(p_supply_hash_conn);
#endif
}

//
//
//
bool DBInterface::OpenDemandConnection(CString ip, CString login, CString pass, CString db)
{
	m_demand_db_address=ip;
	m_demand_db_user=login;
	m_demand_db_password=pass;
	m_demand_db_name=db;

	p_demand_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_demand_conn, ip, login, pass, db, 0, NULL, 0))
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
			log += " DBInterface::OpenDemandConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_demand_conn);
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
bool DBInterface::OpenDemandConnection()
{
	CloseDemandConnection();
	p_demand_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_demand_conn, m_demand_db_address, m_demand_db_user, m_demand_db_password, m_demand_db_name, 0, NULL, 0))
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
			log += " DBInterface::OpenDemandConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_demand_conn);
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
void DBInterface::CloseDemandConnection()
{
	mysql_close(p_demand_conn);
}

//
//	Releases the processing_lock
//	Returns true if the lock was released properly
//
/*
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
*/

//
//
//
//int DBInterface::InsertRawGnutellaDemand(CString& project, int track, CString& query, CString& timestamp)
//{
//	/*
//	project.Replace("\\","_");			// replace the backslash with a "_"
//
//	project.Replace("\'", "\\\'");		// replace the single quote "'"
//	*/
//
//	//truncate the project name if its length is greater than 50
//	if(project.GetLength() > 50)
//	{
//		project.Truncate(50);
//	}
//
//	query.Replace("\\","\\\\");
//	query.Replace("'", "\\'");
//
//	CString table = "DEMAND_TABLE_";
//	table += project;
//	table.Replace('\\','_');			// replace the backslash with _
//	table.Replace('\'', '_');		// replace the single quote "'" with _
//	table.Replace(' ', '_');
//	table.Replace('-', '_');
//	table.Replace('&', '_');
//	table.Replace('!', '_');
//	table.Replace('$', '_');
//	table.Replace('@', '_');
//	table.Replace('%', '_');
//	table.Replace('(', '_');
//	table.Replace(')', '_');
//	table.Replace('+', '_');
//	table.Replace('~', '_');
//	table.Replace('*', '_');
//	table.Replace('.', '_');
//	table.Replace(',', '_');
//	table.Replace('?', '_');
//	table.Replace(':', '_');
//	table.Replace(';', '_');
//	table.Replace('"', '_');
//	table.Replace('/', '_');
//	table.Replace('#', '_');
//
//	int ret;
//	char temp[32];
//	memset(temp, 0, sizeof(temp));
//
//	CString query_str = "INSERT INTO ";
//	query_str += table;
//	query_str += " (track,query,timestamp) VALUES (";
//
//	query_str += itoa(track, temp, 10);
//	query_str += ",'";
//	query_str += query;
//	query_str += "','";
//	query_str += timestamp;
//	query_str += "')";
//
//	ret = mysql_query(p_demand_conn,query_str);
//	
//	if(mysql_error(p_demand_conn)[0] != '\0')
//	{
//		CStdioFile file;
//		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
//		{
//			file.SeekToEnd();
//
//			CString log;
//			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
//			log += " InsertRawGnutellaDemand(...) ";
//			log += query_str;
//			log += "\n";
//			log += "Error: ";
//			log += mysql_error(p_demand_conn);
//			log += "\n\n";
//
//			file.WriteString(log);
//			file.Close();
//
//			if(log.Find("doesn't exist")>=0)
//			{
//				CreateProjectDemandTable(project);
//				mysql_query(p_demand_conn,query_str);
//			}
//
//		}
//		else
//		{
//			DWORD error = GetLastError();
//			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
//		}
//
//		TRACE(mysql_error(p_demand_conn));
//		TRACE("\n");
//		TRACE(query_str);
//		TRACE("\n");
//	}
//	return ret;
//}

//
//
//
int DBInterface::InsertRawGnutellaSupply(/*CString& file_name,UINT file_size,*/ 
			int ip, CString& timestamp, char* sha1,/* GUID& guid,*/ CString& project,int track/*, bool spoof*/)
{

	//file_name.Replace("\\","\\\\");
	//file_name.Replace("'", "\\'");

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


	//UINT guid_index = GetGUIDIndex(guid); //get the guid index from the guid index table;
	int ret;
	char temp[32];
	memset(temp, 0, sizeof(temp));

	CString query = "INSERT IGNORE INTO ";
	query += table;
	query += " (timestamp,sha1,track,ip) VALUES ('";
	query += timestamp;
	query += "','";
	query += sha1;
	query += "',";
	query += itoa(track,temp,10);
	query += ",";
	query += itoa(ip,temp,10);
	query += ")";
	ret = mysql_query(p_supply_conn,query);




	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertRawGnutellaSupply(...) ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
			if(log.Find("doesn't exist")>=0)
			{
				CreateProjectSupplyTable(project);
				mysql_query(p_supply_conn,query);
			}
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");

	}
	return ret;
}

//
//
//
int DBInterface::InsertRawGnutellaSupply(vector<QueryHit>& query_hits)
{
//	file_name.Replace("\\","\\\\");
//	file_name.Replace("'", "\\'");

	//truncate the project name if its length is greater than 50
	CString project = query_hits[0].m_project;
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
	char temp[32];
	memset(temp, 0, sizeof(temp));
	CString query = "INSERT IGNORE INTO ";
	query += table;
	query += " (timestamp,sha1,track,ip) VALUES ";

	for(UINT i=0; i<query_hits.size(); i++)
	{
		query += "('";
		CString timestamp = query_hits[i].m_timestamp.Format("%Y%m%d%H%M%S");
		query += timestamp;
		query += "','";
		CString sha1 = query_hits[i].m_sha1;
		sha1.Replace("\\", "\\\\");
		sha1.Replace("'", "''");
		query += sha1;
		query += "',";
		query += itoa(query_hits[i].m_track,temp,10);
		query += ",";
		query += itoa(query_hits[i].m_ip,temp,10);
		query += ")";
		if(i != query_hits.size()-1)
		{
			query += ",";
		}
	}
	ret = mysql_query(p_supply_conn,query);
	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertRawGnutellaSupply(...) ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
			if(log.Find("doesn't exist")>=0)
			{
				CreateProjectSupplyTable(project);
				mysql_query(p_supply_conn,query);
			}

		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");
	}
	return ret;
}

//
//
//
int DBInterface::InsertRawGnutellaDemand(vector<Query>& project_queries)
{
	if(!project_queries.size())
		return 0;
	//truncate the project name if its length is greater than 50
	if(project_queries[0].m_project.GetLength() > 50)
	{
		project_queries[0].m_project.Truncate(50);
	}

	//query.Replace("\\","\\\\");
	//query.Replace("'", "\\'");

	CString table = "DEMAND_TABLE_";
	table += project_queries[0].m_project;
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
	char temp[32];
	memset(temp, 0, sizeof(temp));

	CString query_str = "INSERT INTO ";
	query_str += table;
	query_str += " (track,query,timestamp) VALUES ";

	for(UINT i=0; i<project_queries.size(); i++)
	{
		query_str += "(";
		query_str += itoa(project_queries[i].m_track, temp, 10);
		query_str += ",'";

		project_queries[i].m_query.Replace("\\","\\\\");
		project_queries[i].m_query.Replace("'", "''");

		query_str += project_queries[i].m_query;
		query_str += "','";
		CString timestamp = project_queries[i].m_timestamp.Format("%Y%m%d%H%M%S");
		query_str += timestamp;
		query_str += "')";
		if(i != project_queries.size()-1)
		{
			query_str += ",";
		}
	}

	ret = mysql_query(p_demand_conn,query_str);
	
	if(mysql_error(p_demand_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertRawGnutellaDemand(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_demand_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();

			if(log.Find("doesn't exist")>=0)
			{
				CreateProjectDemandTable(project_queries[0].m_project);
				mysql_query(p_demand_conn,query_str);
			}

		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
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
	UINT guid_size = mysql_real_escape_string(p_supply_conn, escape_string, (const char*)&guid, sizeof(GUID));

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

	ret = mysql_real_query(p_supply_conn, (const char*)query_buf, query_length);

	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " InsertGUID(...) ";
			log += (const char*)query_buf;
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
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE((const char*)query_buf);
		TRACE("\n");
	}
	delete temp_buf;
	delete query_buf;
	return ret;
}

//
//
//
int DBInterface::CreateProjectDemandTable(CString& project_demand_table)
{
	project_demand_table.Replace('\\','_');			// replace the backslash with _
	project_demand_table.Replace('\'', '_');		// replace the single quote "'" with _
	project_demand_table.Replace(' ', '_');
	project_demand_table.Replace('-', '_');
	project_demand_table.Replace('&', '_');
	project_demand_table.Replace('!', '_');
	project_demand_table.Replace('$', '_');
	project_demand_table.Replace('@', '_');
	project_demand_table.Replace('%', '_');
	project_demand_table.Replace('(', '_');
	project_demand_table.Replace(')', '_');
	project_demand_table.Replace('+', '_');
	project_demand_table.Replace('~', '_');
	project_demand_table.Replace('*', '_');
	project_demand_table.Replace('.', '_');
	project_demand_table.Replace(',', '_');
	project_demand_table.Replace('?', '_');
	project_demand_table.Replace(':', '_');
	project_demand_table.Replace(';', '_');
	project_demand_table.Replace('"', '_');
	project_demand_table.Replace('/', '_');
	project_demand_table.Replace('#', '_');

	CString table = "DEMAND_TABLE_";

	//truncate the project name if its length is greater than 50
	if(project_demand_table.GetLength() > 50)
	{
		project_demand_table.Truncate(50);
	}

	table += project_demand_table;


	int ret;
	//create table IF NOT EXISTS table_name (track tinyint unsigned, query varchar(255), timestamp timestamp, index (track,timestamp), index(timestamp,track))
	CString query = "create table IF NOT EXISTS ";
	query += table;
	query += " (timestamp timestamp,track tinyint unsigned, guid varchar(33), index (timestamp), index(track), primary key(guid))";

	ret = mysql_query(p_demand_conn, query);
	if(mysql_error(p_demand_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " CreateProjectDemandTable(...) ";
			log += query;
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
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_demand_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");
	}
	return ret;
}

//
//
//
int DBInterface::CreateProjectSupplyTable(CString& project_supply_table)
{
	project_supply_table.Replace('\\','_');			// replace the backslash with _
	project_supply_table.Replace('\'', '_');		// replace the single quote "'" with _
	project_supply_table.Replace(' ', '_');
	project_supply_table.Replace('-', '_');
	project_supply_table.Replace('&', '_');
	project_supply_table.Replace('!', '_');
	project_supply_table.Replace('$', '_');
	project_supply_table.Replace('@', '_');
	project_supply_table.Replace('%', '_');
	project_supply_table.Replace('(', '_');
	project_supply_table.Replace(')', '_');
	project_supply_table.Replace('+', '_');
	project_supply_table.Replace('~', '_');
	project_supply_table.Replace('*', '_');
	project_supply_table.Replace('.', '_');
	project_supply_table.Replace(',', '_');
	project_supply_table.Replace('?', '_');
	project_supply_table.Replace(':', '_');
	project_supply_table.Replace(';', '_');
	project_supply_table.Replace('"', '_');
	project_supply_table.Replace('/', '_');
	project_supply_table.Replace('#', '_');

	//truncate the project name if its length is greater than 50
	if(project_supply_table.GetLength() > 50)
	{
		project_supply_table.Truncate(50);
	}

	CString table = "SUPPLY_TABLE_";
	table += project_supply_table;


	int ret;
	//create table IF NOT EXISTS table_name (track tinyint unsigned, query varchar(255), timestamp timestamp, index (track,timestamp), index(timestamp,track))
	CString query = "create table IF NOT EXISTS ";
	query += table;
	query += " (timestamp timestamp,sha1 varchar(32),track tinyint,ip int,primary key (sha1,ip,track))";

	ret = mysql_query(p_supply_conn, query);
	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " CreateProjectSupplyTable(...) ";
			log += query;
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
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
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
	UINT guid_size = mysql_real_escape_string(p_supply_conn, escape_string, (const char*)&guid, sizeof(GUID));

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

	ret = mysql_real_query(p_supply_conn, (const char*)query_buf, query_length);

	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " GetGUIDIndex(...) ";
			log += (const char*)query_buf;
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
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_supply_conn));
		TRACE("\n");
		TRACE((const char*)query_buf);
		TRACE("\n");
		return 0;
	}
	else
	{
		MYSQL_RES *res_set; 
		MYSQL_ROW row; 
		unsigned int i; 

		res_set = mysql_store_result(p_supply_conn);
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
	delete temp_buf;
	delete query_buf;
	return guid_index;
}
