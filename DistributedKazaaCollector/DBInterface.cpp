// DBInterface.cpp: implementation of the DBInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DistributedKazaaCollectorDll.h"
#include "DBInterface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DBInterface::DBInterface()
{

}

DBInterface::~DBInterface()
{

}

void DBInterface::OpenConnection(CString ip, CString login, CString pass, CString db)
{
	p_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_conn, ip, login, pass, db, 0, NULL, 0))
	{
		CString errmsg = "Failed to connect to database: (";
		errmsg += ip;
		errmsg += ") Error: ";
		errmsg += mysql_error(p_conn);
		TRACE(errmsg);
	}
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
//	This function checks to see if there are entries in the 'raw' table from yesterday or before.
//	It also checks to see if it can get the 'processing_lock' from the database.
//
//	Return Value:	0 - found old entries and could get the lock
//					1 - found old entries and could NOT get the lock
//					2 - found no old entries
//
int DBInterface::NeedToProcess(CString raw_data_table_name)
{
	int ret = 2;

	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	//	To process todays data
//	CString query = "select to_days(min(timestamp)) < to_days(now())+1 from ";

	//	To process all previous days data 
	CString query = "select to_days(min(timestamp)) < to_days(now()) from ";

	query += raw_data_table_name;

	mysql_query(p_conn, query);

	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate||CFile::modeNoTruncate||CFile::modeWrite, NULL) != 0)
		{
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " (Select from SearchedMoreIps): ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");
	}

	res=mysql_store_result(p_conn);	// allocates memory
	row=mysql_fetch_row(res);

	CString temp = row[0];
	if (strcmp(temp, "")==0)
		return ret;

	if(strcmp(row[0],"1")==0)	// if there is raw data from a previous day
		ret=1;

	mysql_free_result(res);	// free memory

/*
	if (ret == 1)	// we need to try to get the lock IF there is old data
	{
		// Is the lock free?
		mysql_query(p_conn, "SELECT GET_LOCK(\"processing_lock\", 5)");	// 1 second timeout, if it timesout, just try again
		res=mysql_store_result(p_conn);
		row=mysql_fetch_row(res);

		if(strcmp(row[0],"1")==0)	// if we got the lock, the query will return 1
			ret=0;

		mysql_free_result(res);
	}
*/
	return ret;
}

bool DBInterface::IsNewIp(CString project, CString ip, CString today)
{
	int ret;

	CString query = "SELECT ip FROM SearchedMoreIps where ip = '";
	query += ip;
	query += "' and day like '";
	query += today;
	query += "%' and project = '";
	query += project;
	query += "'";

	ret = mysql_query(p_conn,query);

	if(mysql_error(p_conn)[0] != '\0')
	{
/*	// Logging all of these would be too much.
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate||CFile::modeNoTruncate||CFile::modeWrite, NULL) != 0)
		{
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " (Select from SearchedMoreIps): ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
*/
		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");

		return false;	// if we lose the connection, we don't want to crash,
						// so just return false untill we get the connection back.
	}

	MYSQL_RES *res_set;
	res_set = mysql_store_result(p_conn);

	unsigned int numrows = (unsigned int)mysql_num_rows(res_set);

	mysql_free_result(res_set);	// must call or mem leak

	if (numrows == 0)
	{
		InsertToSearchedIps(project, ip, 0);
		return true;
	}
	else
		return false;
}

int DBInterface::InsertData(CString table, CString project, unsigned int ip, CString user_name, CString title, CString file_name,
							CString file_size, CString hash, CString time_stamp)
{
	file_size.Remove(',');
	file_size.Remove('K');
	file_size.Remove('B');

	project.Replace("\\","_");			// replace the backslash with a "_"
	file_name.Replace("\\","_");
	title.Replace("\\","_");
	user_name.Replace("\\","_");

	project.Replace("\'", "\\\'");		// replace the single quote "'"
	file_name.Replace("\'", "\\\'");
	title.Replace("\'", "\\\'");
	user_name.Replace("\'", "\\\'");

	int ret;
	char temp[20];
	memset(temp, 0, sizeof(temp));

	CString query = "INSERT INTO ";
	query += table;
	query += " (project,ip,user,title,filename,size,hash,timestamp) VALUES ('";

	query += project;
	query += "',";
	query += itoa(ip, temp, 10);
	query += ",'";
	query += user_name;
	query += "','";
	query += title;
	query += "','";
	query += file_name;
	query += "',";
	query += file_size;
	query += ",'";
	query += hash;
	query += "','";
	query += time_stamp;
	query += "')";

	ret = mysql_query(p_conn,query);

	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " (Big Insert): ";
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

int DBInterface::InsertToSearchedIps(CString project, CString ip, int file_count)
{
	int ret;

	char str_file_count[20];
	memset(str_file_count, 0, sizeof(str_file_count));
	itoa(file_count, str_file_count, 10);

	project.Replace("\'", "\\\'");

	CString query = "INSERT INTO SearchedMoreIps (project,ip,day,file_count) VALUES ('";

	query += project;				// project
	query += "','";
	query += ip;					// ip
	query += "',now(),";			// time
	query += str_file_count;		// file count
	query += ")";

	ret = mysql_query(p_conn,query);

	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate||CFile::modeNoTruncate||CFile::modeWrite, NULL) != 0)
		{
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " (Insert into SearchedMoreIps): ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");
	}

	return ret;
}

int DBInterface::UpdateFileCount(CString project, CString ip, CString today, int file_count)
{
	int ret = 0;

	char str_file_count[20];
	memset(str_file_count, 0, sizeof(str_file_count));
	itoa(file_count, str_file_count, 10);
	
	CString query = "UPDATE SearchedMoreIps SET file_count = ";
	query += str_file_count;		// file count
	query += " WHERE project = '";
	query += project;				// project
	query += "' AND ip = '";
	query += ip;					// ip
	query += "' AND day like '";
	query += today;					// today
	query += "%'";
	ret = mysql_query(p_conn,query);

	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate||CFile::modeNoTruncate||CFile::modeWrite, NULL) != 0)
		{
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " (Insert into SearchedMoreIps): ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");
	}

	return ret;
}

void DBInterface::CheckArtistName(CString *artist)
{
	// 2 Pac
	if (_stricmp(*artist, "2pac")==0 ||
		_stricmp(*artist, "2-pac")==0 ||
		_stricmp(*artist, "tu pac")==0 ||
		_stricmp(*artist, "tupac")==0 ||
		_stricmp(*artist, "tupac shakur")==0) *artist = "2 Pac";	
	
	// Jay-Z
	if (_stricmp(*artist, "jayz")==0 ||
		_stricmp(*artist, "jay z")==0 ||
		_stricmp(*artist, "jay - z")==0) *artist = "Jay-Z";

	// Fabolous
	if (_stricmp(*artist, "fabulous")==0) *artist = "Fabolous";

	// These cases are so we don't insert them into the database
	if ( strlen(*artist) == 1 ||
		 strlen(*artist) == 0 ||
		_stricmp(*artist, "unknown")==0 ||
		_stricmp(*artist, "<unknown>")==0 ||
		_stricmp(*artist, "artist")==0 ||
		_stricmp(*artist, "ab+")==0 ||
		_stricmp(*artist, "a+")==0 ||
		_stricmp(*artist, "a++")==0 ||
		_stricmp(*artist, "a +")==0 ||
		_stricmp(*artist, "a ++")==0 ||
		_stricmp(*artist, "coldplay")==0 ||
		_stricmp(*artist, "cold play")==0) *artist = "DO NOT INSERT INTO DATABASE";	
	
	return;
}

int DBInterface::InsertToOtherArtists(MYSQL *conn, CString project, CString artist, CString album, CString day)
{
	int ret;

	project.Replace("\\","\\\\");
	artist.Replace("\\","\\\\");
	album.Replace("\\","\\\\");

	project.Replace("'", "\\'");
	artist.Replace("'", "\\'");
	album.Replace("'", "\\'");
	
	CheckArtistName(&artist);

	if (strcmp(artist, "DO NOT INSERT INTO DATABASE")==0)
		return 0;

	CString query = "SELECT artist,album FROM OtherArtists WHERE project = '";
	query += project;
	query += "' and artist = '";
	query += artist;
	query += "' and album = '";
	query += album;
	query += "' and day like '";
	query += day;
	query += "%'";

	ret = mysql_query(conn,query);

	if(mysql_error(conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate||CFile::modeNoTruncate||CFile::modeWrite, NULL) != 0)
		{
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " (Select from OtherArtists): ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}

		TRACE(mysql_error(conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");
	}

	MYSQL_RES *res_set;
	res_set = mysql_store_result(conn);
	unsigned int numrows = (unsigned int)mysql_num_rows(res_set);
	mysql_free_result(res_set);

	if (numrows == 0)
	{
		// First time we've seen this artist/album today... do INSERT
		query = "INSERT INTO OtherArtists (project,artist,album,count,day) VALUES ('";
		query += project;	// project
		query += "','";
		query += artist;	// artist
		query += "','";
		query += album;		// album
		query += "',1,now())";	// count = 1, current time

		ret = mysql_query(conn,query);

	}
	else
	{
		// We've seen this artist... UPDATE +1
		query = "UPDATE OtherArtists SET count=count+1 WHERE project = '";
		query += project;				// project
		query += "' and artist = '";
		query += artist;				// artist
		query += "' and album = '";
		query += album;					// album
		query += "' and day like '";
		query += day;					// day
		query += "%'";

		ret = mysql_query(conn, query);
	}

	if(mysql_error(conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate||CFile::modeNoTruncate||CFile::modeWrite, NULL) != 0)
		{
			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " (Insert OtherArtists): ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}

		TRACE(mysql_error(conn));
		TRACE("\n");
		TRACE(query);
		TRACE("\n");
	}

	return ret;
}