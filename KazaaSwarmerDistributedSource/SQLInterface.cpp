#include "StdAfx.h"
#include "sqlinterface.h"
#include "mysql.h"

SQLInterface::SQLInterface(void)
{
}

SQLInterface::~SQLInterface(void)
{
}

int SQLInterface::GetCatalogData(vector<FileEntry> * entries, int num)
{
	char number[10];
	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"38.118.160.161","onsystems","ebertsux37","dcdata",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set; 
	MYSQL_ROW row;

	int i = 0;
	int numrows;
	FileEntry temp_entry;

	// Select the Catalog Tracks
	CString query ="Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day)>= to_days(now())-3 AND special = 'C' group by hash Order By 6 desc Limit ";
 
	_itoa(num,number,10);
	query += number;

	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	numrows = (unsigned int)mysql_num_rows(res_set);

	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{ 
		memset(temp_entry.m_project,0,sizeof(temp_entry.m_project));
		memset(temp_entry.m_filename,0,sizeof(temp_entry.m_filename));
		memset(temp_entry.m_hash,0,sizeof(temp_entry.m_hash));
		strcpy(temp_entry.m_project,row[0]);
		temp_entry.m_track = atoi(row[1]);
		strcpy(temp_entry.m_hash,row[2]);
		strcpy(temp_entry.m_filename,row[3]);
		temp_entry.m_size = atoi(row[4]);
		temp_entry.m_count = atoi(row[5]);
		entries->push_back(temp_entry);
	} 
	mysql_free_result(res_set);
	mysql_close(conn);

	return numrows;
	
}

int SQLInterface::GetSpecialData(vector<FileEntry> * entries)
{
	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"38.118.160.161","onsystems","ebertsux37","dcdata",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set; 
	MYSQL_ROW row;

	int i = 0;
	int numrows;
	FileEntry temp_entry;
//	CString query ="Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day) >= to_days(now())-3 AND special = 'T' group by hash Order By 6 desc";
//	CString query ="Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day) = to_days(now())-1 AND project like '%MATCHBOX%' group by hash Order By 6 desc limit 6000";
	CString query ="Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day) >= to_days(now())-3 AND project like '%NORAH JONES%' and (track = 1 or track = 2)group by hash Order By 6 desc limit 6000";
	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	numrows = (unsigned int)mysql_num_rows(res_set);

	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{ 
		memset(temp_entry.m_project,0,sizeof(temp_entry.m_project));
		memset(temp_entry.m_filename,0,sizeof(temp_entry.m_filename));
		memset(temp_entry.m_hash,0,sizeof(temp_entry.m_hash));
		strcpy(temp_entry.m_project,row[0]);
		temp_entry.m_track = atoi(row[1]);
		strcpy(temp_entry.m_hash,row[2]);
		strcpy(temp_entry.m_filename,row[3]);
		temp_entry.m_size = atoi(row[4]);
		temp_entry.m_count = atoi(row[5]);
		entries->push_back(temp_entry);
	} 
	mysql_free_result(res_set);
	mysql_close(conn);

	return numrows;
	
}

int SQLInterface::GetRegularEntries(vector<FileEntry> * entries,int num)
{
	char number[10];
	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"38.118.160.161","onsystems","ebertsux37","dcdata",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set; 
	MYSQL_ROW row;

	int i = 0;
	int numrows;
	FileEntry temp_entry;

	// Select the Catalog Tracks
	CString query ="Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day)>= to_days(now())-3 AND special = 'F' group by hash Order By 6 desc Limit ";
 
	_itoa(num,number,10);
	query += number;

	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	numrows = (unsigned int)mysql_num_rows(res_set);

	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{ 
		memset(temp_entry.m_project,0,sizeof(temp_entry.m_project));
		memset(temp_entry.m_filename,0,sizeof(temp_entry.m_filename));
		memset(temp_entry.m_hash,0,sizeof(temp_entry.m_hash));
		strcpy(temp_entry.m_project,row[0]);
		temp_entry.m_track = atoi(row[1]);
		strcpy(temp_entry.m_hash,row[2]);
		strcpy(temp_entry.m_filename,row[3]);
		temp_entry.m_size = atoi(row[4]);
		temp_entry.m_count = atoi(row[5]);
		entries->push_back(temp_entry);
	} 
	mysql_free_result(res_set);
	mysql_close(conn);

	return numrows;
	
}


int SQLInterface::GetNonSpecialEntries(vector<FileEntry> * entries,int num)
{
	char number[10];
	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"38.118.160.161","onsystems","ebertsux37","dcdata",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set; 
	MYSQL_ROW row;

	int i = 0;
	int numrows;
	FileEntry temp_entry;

	// Select the Catalog Tracks
	CString query ="Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day) = to_days(now())-1 AND special != 'T' group by hash Order By 6 desc Limit ";
 
	_itoa(num,number,10);
	query += number;

	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	numrows = (unsigned int)mysql_num_rows(res_set);

	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{ 
		memset(temp_entry.m_project,0,sizeof(temp_entry.m_project));
		memset(temp_entry.m_filename,0,sizeof(temp_entry.m_filename));
		memset(temp_entry.m_hash,0,sizeof(temp_entry.m_hash));
		strcpy(temp_entry.m_project,row[0]);
		temp_entry.m_track = atoi(row[1]);
		strcpy(temp_entry.m_hash,row[2]);
		strcpy(temp_entry.m_filename,row[3]);
		temp_entry.m_size = atoi(row[4]);
		temp_entry.m_count = atoi(row[5]);
		entries->push_back(temp_entry);
	} 
	mysql_free_result(res_set);
	mysql_close(conn);

	return numrows;
	
}