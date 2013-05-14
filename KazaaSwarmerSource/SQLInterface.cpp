#include "StdAfx.h"
#include "sqlinterface.h"
#include "mysql.h"
#include "MusicProject.h"

SQLInterface::SQLInterface(void)
{
}

SQLInterface::~SQLInterface(void)
{
}

bool SQLInterface::GetData(vector<FileEntry> *entries, int num, int numcatalog)// vector<MusicProject> projects)
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
	int minmaxiter = 0;
//	char intchar[10];

	FileEntry temp_entry;
	/*
	for (int p = 0; p < (int)projects.size(); p++)
	{
		CString query =("Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day)=to_days(now()-interval 1 day) AND project = '");
		query += projects[p].m_projectname;
		query += ("' AND track = ");
		_itoa(projects[p].m_track,intchar,10);
		query += intchar;
		query += (" group by hash Order By 6 desc");

		mysql_query(conn,query);
		sprintf(error,"Error: %s",mysql_error(conn));
		res_set = mysql_store_result(conn);
		sprintf(error,"Error: %s",mysql_error(conn));
		unsigned int numrows = (unsigned int)mysql_num_rows(res_set);

		minmaxiter = 0;

		while ((row = mysql_fetch_row(res_set)) != NULL) 
		{ 
			if (minmaxiter < projects[p].m_maxswarms)
			{
				strcpy(temp_entry.m_project,row[0]);
				temp_entry.m_track = atoi(row[1]);
				strcpy(temp_entry.m_hash,row[2]);
				*/
				/*
				strcpy(temp_entry.m_filename,"(");
				_itoa(i,intchar,10);
				strcat(temp_entry.m_filename,intchar);
				strcat(temp_entry.m_filename,")");
				*/
	/*
				strcpy(temp_entry.m_filename,row[3]);
				temp_entry.m_size = atoi(row[4]);
				temp_entry.m_count = atoi(row[5]);
				entries->push_back(temp_entry);
				i++;
				minmaxiter++;
			}
			else
			{
				break;
			}
		} 
		mysql_free_result(res_set);


	}
*/

	// Select the Special tracks First
	CString query ="Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day)=to_days(now()-interval 1 day) AND special = 'T' group by hash Order By 6 desc";
 
	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	unsigned int numrows = (unsigned int)mysql_num_rows(res_set);



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
		i++;
	} 
	mysql_free_result(res_set);

	// Select the Catalog Tracks
	query ="Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day)=to_days(now()-interval 1 day) AND special = 'C' group by hash Order By 6 desc Limit ";
 
	_itoa(numcatalog,number,10);
	query += number;

	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	numrows = (unsigned int)mysql_num_rows(res_set);

	// Don't increment i since this is a special set outside of the standard number
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

	query ="Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day)=to_days(now()-interval 1 day) AND special = 'F' group by hash Order By 6 desc Limit ";
	// Subtract the special ones we already did
	num = num - i;
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
		/*
		strcpy(temp_entry.m_filename,"(");
		_itoa(i,intchar,10);
		strcat(temp_entry.m_filename,intchar);
		strcat(temp_entry.m_filename,")");
		*/
		strcpy(temp_entry.m_filename,row[3]);
		temp_entry.m_size = atoi(row[4]);
		temp_entry.m_count = atoi(row[5]);
		entries->push_back(temp_entry);
		i++;
	} 
	mysql_free_result(res_set);
	mysql_close(conn);

	// If we have a seriously small amount of data it's no good
	int size = (int)entries->size();
	if (i < (num/2))
	{
		return false;
	}
	return true;
	
}