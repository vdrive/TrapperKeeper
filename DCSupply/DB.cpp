#include "StdAfx.h"
#include "db.h"
#include "Winsock2.h"	//required to convert ip address to long

DB::DB(void)
{
	
}

DB::~DB(void)
{
}
//pass in ip,login,password and database name to connect to
//returns: true if successfully connected, false otherwise
bool DB::Connect(CString ip, CString login, CString pass, CString db)
{
	p_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_conn, ip, login, pass, db, 0, NULL, 0))
	{
		CString errmsg = "Failed to connect to database: (";
		errmsg += ip;
		errmsg += ") Error: ";
		errmsg += mysql_error(p_conn);
		TRACE(errmsg);
		return 0;	//error connecting
	}
	return 1;	//connected successfully
}
//closes connection to database
void DB::Disconnect()
{
	mysql_close(p_conn);
}
//
//
int DB::InsertData(CString project,CString track_name,int track_num,CString file_name,CString file_size, CString user,CString ip_address,CString table)
{
	int ret;
	//char temp[200];
	//memset(temp, 0, sizeof(temp));
	//unsigned long ip=inet_addr(ip_address);

	file_size.Remove(',');
	file_size.Remove('K');
	file_size.Remove('B');

	project.Replace("\\","_");			// replace the backslash with a "_"
	file_name.Replace("\\","_");


	project.Replace("\'", "\\\'");		// replace the single quote "'"
	file_name.Replace("\'", "\\\'");


	CString query = "insert into ";
	query += table;
	query += " (project,ip,user,trackname,filename,filesize,timestamp,track_number) VALUES ('";

	query += project;
	query += "',inet_aton('";
	query += ip_address;
	query += "')";
	query += ",'";
	query += user;
	query += "','";
	query += track_name;
	query +="','";
	query += file_name;
	query += "','";
	query += file_size;
	query += "',";
	query += "now()";
	query += ",";
	query += itoa(track_num,m_track_number,10);
	query += ")";
	ret = mysql_query(p_conn,query);
	return ret;
}

//insert data to the dc master
void DB::InsertDCMasterData()
{
	MYSQL *p_dc_conn;	//connect to dc master
	MYSQL_RES *res_set; 
	MYSQL_ROW row;
	int first_pass = 0;	//variable that determines first pass
	unsigned int i;

	//extract data for today
	mysql_query(p_conn,"select timestamp,project,track_number,count(*) from direct_connect_supply where timestamp = now()- interval 1 day group by project,track_number");
	res_set = mysql_store_result(p_conn);

	string batch_query;
	batch_query +="insert into direct_connect_supply(date,project,track,supply)";

	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{
		if(first_pass==0)
		{
			batch_query += " values(";
			first_pass++;
		}
		else
		{
			batch_query += ",(";
		}

		for (i=0; i<mysql_num_fields(res_set); i++) 
		{
			//first field, filename
			if(i<=2)
			{
				batch_query += "\"";
				batch_query += row[i];
				batch_query += "\",";
			}		
			//second field,filesize
			else
			{
				batch_query += "\"";
				batch_query += row[i];
				batch_query += "\"";
				batch_query += ")";
			}
		}
	}

	//batch_query += ")";
	p_dc_conn = mysql_init(NULL);
	
	//initialize dc master connection
	if(!mysql_real_connect(p_dc_conn, "38.118.160.161", "onsystems", "ebertsux37", "dcdata", 0, NULL, 0))
	{
		CString errmsg = "Failed to connect to database: ";
		errmsg += mysql_error(p_conn);
		TRACE(errmsg);
		CFile file_writedatabase("c:\\DCMaster Info\\DCScanner\\log.txt",CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
		CTime time = CTime::GetCurrentTime();
		CString status;
		status = "Failed to connect to database\n";
		status += time.Format("%A, %B %d, %Y %X");
		//save checksums
		file_writedatabase.Write(status,(UINT)status.GetLength());
		return;	//error connecting
	}

	//create long insert string
	//do batch insert query
	mysql_query(p_dc_conn,batch_query.c_str());
	//mysql_query(p_dc_conn,"insert into direct_connect_supply(date,project,track,supply) values(\"2003-12-28\",\"test\",\"3\",\"44\"");
	//disconnect from database
	mysql_close(p_dc_conn);

	CFile file_writedatabase("c:\\DCMaster Info\\DCScanner\\log.txt",CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
	CTime time = CTime::GetCurrentTime();
	CString status;
	status = "Writing to database\n";
	status += time.Format("%A, %B %d, %Y %X");
	//save checksums
	file_writedatabase.Write(status,(UINT)status.GetLength());
	file_writedatabase.Write(batch_query.c_str(),(UINT)strlen(batch_query.c_str()));
}