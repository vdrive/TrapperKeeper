#include "StdAfx.h"
#include "sqlinterface.h"
#include "mysql.h"

SQLInterface::SQLInterface(void)
{
}

SQLInterface::~SQLInterface(void)
{
}

void SQLInterface::GetData(vector<FileEntry> *entries, int num)
{

	char number[10];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"66.28.91.224","onsystems","ebertsux37","dcdata",0,NULL,0);

	CString query ="Select * from hashcount where to_days(timestamp)=to_days(now()-interval 1 day) Order By (count) desc Limit ";
	_itoa(num,number,10);
	query += number;
	FileEntry temp_entry;
	
	MYSQL_RES *res_set; 
	MYSQL_ROW row; 
	mysql_query(conn,query);
	res_set = mysql_store_result(conn);
	unsigned int numrows = (unsigned int)mysql_num_rows(res_set);

	while ((row = mysql_fetch_row(res_set)) != NULL) 
	{ 
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
	
}