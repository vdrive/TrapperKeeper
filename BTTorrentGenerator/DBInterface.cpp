#include "StdAfx.h"
#include "DBInterface.h"

DBInterface::DBInterface()
{
}

DBInterface::~DBInterface()
{

}

MYSQL *DBInterface::getConnectionHandle()
{
	return conn;
}



bool DBInterface::OpenConnection(CString ip, CString login, CString pass, CString db)
{
	if (!(conn = mysql_init(NULL)))
	{
		TRACE( "mysql_init() fails" );
	}

	if (!mysql_real_connect(conn, ip, login, pass, db, 0, NULL, 0))
	{
		 CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " DBInterface::OpenSupplyConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(conn);
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
 int DBInterface::executeQuery(string query)
 {
	int ret = mysql_real_query(conn, query.c_str(), (unsigned int)query.length());
	if(mysql_error(conn)[0] != '\0')
	{
			string log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " Createbittorrent_fileTable(...) ";
			log += query;
			log += "\n";
			log += "Error: ";
			log += mysql_error(conn);
			log += "\n\n";
			TRACE( log.c_str() );
	}
	return ret;	
}


void DBInterface::CloseConnection()
{
	mysql_close(conn);
}
