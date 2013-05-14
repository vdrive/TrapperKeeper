#include "StdAfx.h"
#include "db.h"
#include "Winsock2.h"	//required to convert ip address to long

DB::DB(void)
{

}

//disconnect from database
DB::~DB(void)
{
	mysql_close(p_conn);
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
int DB::InsertData(CString project,CString track_name,CString file_name,CString file_size, CString user,CString ip_address,CString table)
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
	query += " (project,ip,user,trackname,filename,filesize,timestamp) VALUES ('";

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
	query += ")";
	ret = mysql_query(p_conn,query);
	return ret;
}
//gets the date from the database and passes it to date variable
string DB::GetDate()
{
	string date;
	MYSQL_RES *res_set; 
	MYSQL_ROW row;

	CString query = "Select date_format(now(),'%m-%d-%Y')";
	mysql_query(p_conn,query);
	res_set = mysql_store_result(p_conn);

	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{
		for (unsigned int i=0; i<mysql_num_fields(res_set); i++) 
		{
			if(i == 0)
			{
				date = row[i];
			}
		}
	}
	mysql_free_result(res_set);
	return date;

}
//	Extract data from the database in order to do spoofing
//
int DB::ExtractData(CString project,CString track_name,CString table, vector<SpoofStruct> &v_spoofs)
{
	//vector<SpoofStruct> spoof;
	int success;
	MYSQL_RES *res_set; 
	MYSQL_ROW row; 

	CString query = "select filename,filesize,user from ";
	query += table;
	query += " where project = \"";
	query += project;
	query += "\"";
	if(track_name.GetLength()!=0)
	{
		query += " and trackname = \"";
		query += track_name;
		query += "\"";
	}
	query += " limit 50";
	//success = mysql_query(p_conn,query);
	if(mysql_query(p_conn,query)!=0)		//checks if query is valid
	{
		TRACE("ERROR: %s",mysql_error(p_conn));
		::AfxMessageBox(mysql_error(p_conn));
		return 0;
	}
	success = 1;
	res_set = mysql_store_result(p_conn);

	SpoofStruct spoof_struct;
	//v_spoofs.clear();	//clear all the content of the vector
	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{
		for (unsigned int i=0; i<mysql_num_fields(res_set); i++) 
		{
			//first field, filename
			if(i == 0)
			{
				spoof_struct.m_filename = row[i];
			}		
			//second field,filesize
			else if(i == 1)
			{
				spoof_struct.m_filesize = atoi(row[i]);		//convert to integer
				if(spoof_struct.m_filesize<0)
				{
					spoof_struct.m_filesize = 56738309;
				}
			}
			//third field, user
			else if(i == 2)
			{

				CString temp = row[i];
				//error check if " " exists
				if(temp.Find(" ",0)>0)
				{
					temp = temp.Left(temp.Find(" ",0));
				}
				spoof_struct.m_user = temp;
			}
			//fourth field,ip
			//else if(i == 3)
			//{
			//	spoof_struct.ip = "1.2.3.4";	//changed 2004-3-03 made constant
			//}
		}
 		v_spoofs.push_back(spoof_struct);	//add object to vector
	}
	mysql_free_result(res_set);
	return success;
}

void DB::SingleTrackDataExists(string project,int track)
{
	int count = 1;
	MYSQL_RES *res_set; 
	MYSQL_ROW row;
	CString query;
	char buffer[300];	//itoa buffer

	query = "Select count(*) from direct_connect_requests where timestamp= now() and project = \"";
	query += project.c_str();
	query += "\" ";
	query += " and track = ";
	query += itoa(track,buffer,10);
	mysql_query(p_conn,query);
	//res_set = mysql_store_result(p_conn);
	res_set = mysql_store_result(p_conn);
	//check whether the value is greater than zero
	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{
		count = atoi(row[0]);	//number of results
	}

	if(count==0)
	{
		query = "";
		query = "Insert into direct_connect_requests(project,track,demand,timestamp) values(\"";
		query += project.c_str();
		query += "\",";
		query += itoa(track,buffer,10);
		query += ",";
		//query += "\"";
		query += "0,now())";
		mysql_query(p_conn,query);
	}
	mysql_free_result(res_set);
}
//checks whether we have spoof data for current project
//if yes, it will just return;
//if no, it will insert data for today
void DB::DataExists(string project)
{
	int count = 1;
	MYSQL_RES *res_set; 
	MYSQL_ROW row;
	CString query;

	query = "Select count(*) from direct_connect_spoofs where timestamp= now() and project = \"";
	query += project.c_str();
	query += "\"";
	mysql_query(p_conn,query);
	//res_set = mysql_store_result(p_conn);
	res_set = mysql_store_result(p_conn);
	//check whether the value is greater than zero
	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{
		count = atoi(row[0]);	//number of results
	}

	if(count==0)
	{
		query = "";
		query = "Insert into direct_connect_spoofs(project,spoofs,timestamp) values(\"";
		query += project.c_str();
		query += "\"";
		query += ",0,now())";
		mysql_query(p_conn,query);
		query = "";
		query = "Insert into direct_connect_requests(project,demand,track,timestamp) values(\"";
		query += project.c_str();
		query += "\"";
		query += ",0,0,now())";
		mysql_query(p_conn,query);
		query = "";
		query = "Insert into direct_connect_decoys(project,decoys,timestamp) values(\"";
		query += project.c_str();
		query += "\"";
		query += ",0,now())";
		mysql_query(p_conn,query);

	}
	mysql_free_result(res_set);
}

//
//Increment the number of spoofs that were sent
void DB::IncrementDBSpoofs(string project,int spoofs)
{

	CString query;
	char buffer[300];	//hold int as string
	query = "UPDATE direct_connect_spoofs set spoofs = spoofs + ";
	query +=itoa(spoofs,buffer,10);
	query += " where project = \"";
	query += project.c_str();
	query += "\" and timestamp= now()";
	//do the update query
	mysql_query(p_conn,query);
}

//
//Increment the number of decoys that were sent
void DB::IncrementDBDecoys(string project,int decoys)
{
	CString query;
	char buffer[300];	//hold int as string
	query = "UPDATE direct_connect_decoys set decoys = decoys + ";
	query +=itoa(decoys,buffer,10);
	query += " where project = \"";
	query += project.c_str();
	query += "\" and timestamp= now()";
	//do the update query
	mysql_query(p_conn,query);
}

//
//Increment the number of requests that were sent
void DB::IncrementDBRequests(string project,int requests,int track)
{

	CString query;
	char buffer[300];	//hold int as string
	query = "UPDATE direct_connect_requests ";
	query += " set demand = demand + ";
	query +=itoa(requests,buffer,10);
	query += " where project = \"";
	query += project.c_str();
	query += "\" and timestamp= now() ";
	query += "and track=\"";
	query += itoa(track,buffer,10);
	query += "\"";
	//do the update query
	mysql_query(p_conn,query);
}

//
//returns the filesize for the corresponding filename
string DB::GetFilesize(string filename)
{
	MYSQL_RES *res_set;
	MYSQL_ROW row;

	CString query;	//string for query
	
	string filesize;	//string for filesize we are tyring to find
	query = "select filesize from direct_connect_supply where filename=\"";
	query += filename.c_str();
	query += "\" limit 1";
	mysql_query(p_conn,query);
	res_set = mysql_store_result(p_conn);
	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{
		filesize = row[0];	//number of results
	}
	mysql_free_result(res_set);
	return filesize;
}

//insert data to the dc master
void DB::InsertDCMasterData()
{
	MYSQL *p_dc_conn;	//connect to dc master
	MYSQL_RES *res_set; 
	MYSQL_ROW row;
	int first_pass = 0;	//variable that determines first pass
	unsigned int i;

	//////////////spoofs/////////////////
	//extract data for today
	mysql_query(p_conn,"select project,spoofs,timestamp from direct_connect_spoofs where timestamp = now()- interval 1 day");
	res_set = mysql_store_result(p_conn);

	string batch_query;
	string requests_batch_query;
	string decoys_batch_query;

	batch_query +="insert into direct_connect_spoofs(project,spoofs,date)";

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
			if(i<2)
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

	/////////////////////// demand	////////////////////////////
	//extract data for today
	mysql_query(p_conn,"select project,demand,track,timestamp from direct_connect_requests where timestamp = now()- interval 1 day");
	res_set = mysql_store_result(p_conn);

	requests_batch_query +="insert into direct_connect_demand(project,demand,track,date)";
	first_pass=0;
	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{
		if(first_pass==0)
		{
			requests_batch_query += " values(";
			first_pass++;
		}
		else
		{
			requests_batch_query += ",(";
		}

		for (i=0; i<mysql_num_fields(res_set); i++) 
		{
			//first field, filename
			if(i<3)
			{
				requests_batch_query += "\"";
				requests_batch_query += row[i];
				requests_batch_query += "\",";
			}		
			//fourth field,date
			else
			{
				requests_batch_query += "\"";
				requests_batch_query += row[i];
				requests_batch_query += "\"";
				requests_batch_query += ")";
			}
		}
	}

	/////////////////////// decoys	////////////////////////////
	//extract data for today
	mysql_query(p_conn,"select project,decoys,timestamp from direct_connect_decoys where timestamp = now()- interval 1 day");
	res_set = mysql_store_result(p_conn);

	first_pass = 0;
	decoys_batch_query +="insert into direct_connect_decoys(project,decoys,date)";
	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{
		if(first_pass==0)
		{
			decoys_batch_query += " values(";
			first_pass++;
		}
		else
		{
			decoys_batch_query += ",(";
		}

		for (i=0; i<mysql_num_fields(res_set); i++) 
		{
			//first field, filename
			if(i<2)
			{
				decoys_batch_query += "\"";
				decoys_batch_query += row[i];
				decoys_batch_query += "\",";
			}		
			//fourth field,date
			else
			{
				decoys_batch_query += "\"";
				decoys_batch_query += row[i];
				decoys_batch_query += "\"";
				decoys_batch_query += ")";
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
	mysql_query(p_dc_conn,requests_batch_query.c_str());
	mysql_query(p_dc_conn,decoys_batch_query.c_str());
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