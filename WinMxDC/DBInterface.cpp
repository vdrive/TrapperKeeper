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

//
//
//
bool DBInterface::OpenSupplyConnection(CString ip, CString login, CString pass, CString db)
{
	p_supply_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_supply_conn, ip, login, pass, db, 0, NULL, 0))
	{
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
		CloseSupplyConnection();
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
}

//
//
//
int DBInterface::InsertRawSupply(vector<QueryHit>& query_hits)
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
	CString query = "INSERT INTO ";
	query += table;
	query += " (filename,filesize,hash,ip,port,search_id,timestamp,track,bitrate,frequency,duration) VALUES ";

	for(UINT i=0; i<query_hits.size(); i++)
	{
		query += "('";
		CString filename = query_hits[i].m_filename;
		filename.Replace("\\", "\\\\");
		filename.Replace("'", "\\'");
		query+= filename;
		query += "',";
		query += ultoa(query_hits[i].m_file_size, temp, 10);
		query += ",'";
		
		char hash[41];
		memset(&hash,0,sizeof(hash));
		for (int j = 0; j < 16; j++)
		{
			sprintf((char *)&hash[j*2],"%02x",((int)query_hits[i].m_hash[j])&0xFF);
		}
		query += hash;
		query += "',";
		query += itoa(query_hits[i].m_ip,temp,10);
		query += ",";
		query += itoa(query_hits[i].m_port,temp,10);
		query += ",";
		query += itoa(query_hits[i].m_search_id,temp,10);
		query += ",'";
		CString timestamp = query_hits[i].m_timestamp.Format("%Y%m%d%H%M%S");
		query += timestamp;
		query += "',";
		query += itoa(query_hits[i].m_track,temp,10);
		query += ",";
		query += itoa(query_hits[i].m_bitrate,temp,10);
		query += ",";
		query += itoa(query_hits[i].m_sample_frequency,temp,10);
		query += ",";
		query += itoa(query_hits[i].m_duration,temp,10);
		query += ")";
		if(i != query_hits.size()-1)
			query += ",";
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
			log += " InsertRawSupply(...) ";
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
int DBInterface::InsertRawSupply(QueryHit& query_hits)
{
//	file_name.Replace("\\","\\\\");
//	file_name.Replace("'", "\\'");

	//truncate the project name if its length is greater than 50
	CString project = query_hits.m_project;
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
	CString query = "INSERT INTO ";
	query += table;
	query += " (filename,filesize,hash,ip,port,search_id,timestamp,track,bitrate,frequency,duration) VALUES ";

		query += "('";
		CString filename = query_hits.m_filename;
		filename.Replace("\\", "\\\\");
		filename.Replace("'", "\\'");
		query+= filename;
		query += "',";
		query += ultoa(query_hits.m_file_size, temp, 10);
		query += ",'";
		
		char hash[41];
		memset(&hash,0,sizeof(hash));
		for (int j = 0; j < 16; j++)
		{
			sprintf((char *)&hash[j*2],"%02x",((int)query_hits.m_hash[j])&0xFF);
		}
		query += hash;
		query += "',";
		query += ultoa(query_hits.m_ip,temp,10);
		query += ",";
		query += itoa(query_hits.m_port,temp,10);
		query += ",";
		query += itoa(query_hits.m_search_id,temp,10);
		query += ",'";
		CString timestamp = query_hits.m_timestamp.Format("%Y%m%d%H%M%S");
		query += timestamp;
		query += "',";
		query += itoa(query_hits.m_track,temp,10);
		query += ",";
		query += itoa(query_hits.m_bitrate,temp,10);
		query += ",";
		query += itoa(query_hits.m_sample_frequency,temp,10);
		query += ",";
		query += itoa(query_hits.m_duration,temp,10);
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
			log += " InsertRawSupply(...) ";
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
	query += " (filename varchar(255),filesize int unsigned,hash varchar(33),ip int,port smallint unsigned,search_id int unsigned,timestamp timestamp,track tinyint unsigned,bitrate smallint, frequency int, duration int,index(search_id, timestamp,track),index(timestamp,search_id,track),index(hash,track),index(hash,track),index(track,hash))";

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
/*
CString DBInterface::CreateFakeSupplyTable(const char* project)
{
	CString project_supply_table = project;
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
	query += " (timestamp timestamp,track tinyint unsigned, supply int unsigned,index(timestamp,track))";

	ret = mysql_query(p_supply_conn, query);
	if(mysql_error(p_supply_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " CreateFakeSupplyTable(...) ";
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
	return table;
}

//
//
//

int DBInterface::InsertFakeSupplies(VirtualProjectStatusListCtrlItem& data)
{
	CString table = CreateFakeSupplyTable(data.m_project_name.c_str());
	int ret;
	char temp[32];
	memset(temp, 0, sizeof(temp));
	CString query = "INSERT INTO ";
	query += table;
	query += " (track,supply) VALUES ";

	for(UINT i=0; i<data.v_track_fake_query_hit_count.size(); i++)
	{
		query += "(";
		query += ultoa(i, temp, 10);
		query += ",";
		query += ultoa(data.v_track_fake_query_hit_count[i],temp,10);
		query += ")";
		if(i != data.v_track_fake_query_hit_count.size()-1)
			query += ",";
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
			log += " InsertFakeSupplies(...) ";
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
*/