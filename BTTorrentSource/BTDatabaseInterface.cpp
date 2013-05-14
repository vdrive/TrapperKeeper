#include "StdAfx.h"
#include ".\btdatabaseinterface.h"
#include "mysql.h"


BTDatabaseInterface::BTDatabaseInterface(void)
{
}

BTDatabaseInterface::~BTDatabaseInterface(void)
{
}


void BTDatabaseInterface::GetIPInfo(vector<TorrentFileData> * tfiles)
{
	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set; 
	MYSQL_ROW row;

	int i = 0;
	int numrows;
	TorrentFileData temp_entry;

	
	//"Select project,track,hash,filename,size,max(count),day from hashcount where to_days(day)>= to_days(now())-3 AND special = 'C' group by hash Order By 6 desc Limit ";
	CString query ="Select torrent_id.hash, announce_list.url, torrent_id.hash_id from torrent_id, announce_list WHERE to_days(torrent_id.time) >= to_days(now())-1 AND torrent_id.hash_id = announce_list.hash_id";

	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	if (mysql_errno(conn) == 0)
	{
		numrows = (unsigned int)mysql_num_rows(res_set);

		TorrentFileData tempdata;

		while ((row = mysql_fetch_row(res_set)) != NULL) 
		{

			tempdata.m_hash=row[0];
			tempdata.m_announce_URL=row[1];
			tempdata.m_torrent_id = atoi(row[2]);
			tfiles->push_back(tempdata);
		}
	}
	/*
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
	*/
	mysql_free_result(res_set);
	mysql_close(conn);

}

void BTDatabaseInterface::InsertIPs(BTIPList iplist)
{

	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));



	int i = 0;
	vector<BTIP> ips = iplist.GetIps();


	for (i = 0; i < iplist.m_num_ips; i ++)
	{
		
		char query[1024+1];
//		sprintf(query,"Insert into torrent_hash_ips values('%s',inet_aton('%s'),%d,now())",ips[i].m_id,ips[i].m_ip,ips[i].m_port);
		sprintf(query,"Insert into torrent_ips values(%d,inet_aton('%s'),%d,now())",ips[i].m_torrent_id,ips[i].m_ip,ips[i].m_port);
		mysql_query(conn,query);
		sprintf(error,"Error: %s",mysql_error(conn));
		if (mysql_errno(conn) != 0)
		{
//			sprintf(query,"Update torrent_hash_ips set time = now() where hash = '%s' AND ip = inet_aton('%s') AND port = %d",ips[i].m_id,ips[i].m_ip,ips[i].m_port);
			sprintf(query,"Update torrent_ips set time = now() where hash_id = %d AND ip = inet_aton('%s') AND port = %d",ips[i].m_torrent_id,ips[i].m_ip,ips[i].m_port);
			mysql_query(conn,query);
			sprintf(error,"Error: %s",mysql_error(conn));
		}

	}

	mysql_close(conn);

}

void BTDatabaseInterface::GetFullTorrentInfo(vector<TorrentFileData> * tfiles)
{

	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set, *res_set2; 
	MYSQL_ROW row, row2;

	int i = 0;
	int numrows;
	TorrentFileData temp_entry;



	CString query ="Select torrent_id.hash, announce_list.url, torrent_info.name, torrent_info.piece_length, torrent_id.hash_id from torrent_id, announce_list, torrent_info WHERE to_days(torrent_id.time) = to_days(now()) AND torrent_id.hash_id = announce_list.hash_id AND torrent_id.hash_id = torrent_info.hash_id";

	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	if (mysql_errno(conn) == 0)
	{
		numrows = (unsigned int)mysql_num_rows(res_set);

		TorrentFileData tempdata;

		while ((row = mysql_fetch_row(res_set)) != NULL) 
		{

			tempdata.m_hash=row[0];
			tempdata.m_announce_URL=row[1];
			tempdata.m_name = row[2];
			tempdata.m_piece_length = atoi(row[3]);
			tempdata.m_torrent_id = atoi(row[4]);

			char query2[1024+1];
			sprintf(query2,"select sha1, length, path from Torrent_file_info where hash_id = %d",tempdata.m_torrent_id);
			mysql_query(conn,query2);
			sprintf(error,"Error: %s",mysql_error(conn));
			res_set2 = mysql_store_result(conn);
			sprintf(error,"Error: %s",mysql_error(conn));
			if (mysql_errno(conn) == 0)
			{
				numrows = (unsigned int)mysql_num_rows(res_set2);

				FileInfo tempinfo;

				while ((row2 = mysql_fetch_row(res_set2)) != NULL) 
				{
					tempinfo.m_md5 = row2[0];
					tempinfo.m_length = atoi(row2[1]);
					tempinfo.m_pathname = row2[2];
					tempdata.v_files.push_back(tempinfo);

				}
			}

			mysql_free_result(res_set2);

			sprintf(query2,"select piece_hash from Torrent_pieces where hash_id = %d Order by piece_number",tempdata.m_torrent_id);
			mysql_query(conn,query2);
			sprintf(error,"Error: %s",mysql_error(conn));
			res_set2 = mysql_store_result(conn);

			if (mysql_errno(conn) == 0)
			{
				string temphash;
				numrows = (unsigned int)mysql_num_rows(res_set2);

				FileInfo tempinfo;

				while ((row2 = mysql_fetch_row(res_set2)) != NULL) 
				{
					temphash = row2[0];
					tempdata.v_piece_hashes.push_back(temphash);

				}
			}

			mysql_free_result(res_set2);
			tfiles->push_back(tempdata);
		}
	}

	mysql_free_result(res_set);
	mysql_close(conn);

}


void BTDatabaseInterface::GetTorrentInfoSubset(vector<TorrentFileData> * tfiles, vector<HashIdPair> torrent_ids, vector<BTIPList> * vips)
{

	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set, *res_set2; 
	MYSQL_ROW row, row2;

	int i = 0;
	int numrows;
	TorrentFileData temp_entry;


	char query[1024+1];
	for (int i = 0; i< (int)torrent_ids.size(); i++)
	{
	sprintf(query,"Select torrent_id.hash, announce_list.url, torrent_info.name, torrent_info.piece_length, torrent_id.hash_id from torrent_id, announce_list, torrent_info WHERE torrent_id.hash_id = %d AND torrent_id.hash_id = announce_list.hash_id AND torrent_id.hash_id = torrent_info.hash_id",torrent_ids[i].torrent_id);

	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	if (mysql_errno(conn) == 0)
	{
		numrows = (unsigned int)mysql_num_rows(res_set);

		TorrentFileData tempdata;

		while ((row = mysql_fetch_row(res_set)) != NULL) 
		{

			tempdata.Clear();
			tempdata.m_hash=row[0];
			tempdata.m_announce_URL=row[1];
			tempdata.m_name = row[2];
			tempdata.m_piece_length = atoi(row[3]);
			tempdata.m_torrent_id = atoi(row[4]);

			char query2[1024+1];
			sprintf(query2,"select sha1, length, path from Torrent_file_info where hash_id = %d",tempdata.m_torrent_id);
			mysql_query(conn,query2);
			sprintf(error,"Error: %s",mysql_error(conn));
			res_set2 = mysql_store_result(conn);
			sprintf(error,"Error: %s",mysql_error(conn));
			if (mysql_errno(conn) == 0)
			{
				numrows = (unsigned int)mysql_num_rows(res_set2);

				FileInfo tempinfo;

				while ((row2 = mysql_fetch_row(res_set2)) != NULL) 
				{
					tempinfo.m_md5 = row2[0];
					tempinfo.m_length = atoi(row2[1]);
					tempinfo.m_pathname = row2[2];
					tempdata.v_files.push_back(tempinfo);

				}
			}

			mysql_free_result(res_set2);
			//sprintf(query2,"select hash_id, ip, port from torrent_ips where hash_id = %d",tempdata.m_torrent_id);
			sprintf(query2,"select torrent_ips.hash_id, torrent_ips.ip, torrent_ips.port, torrent_id.hash from torrent_ips, torrent_id where torrent_id.hash_id = %d and torrent_id.hash_id = torrent_ips.hash_id",tempdata.m_torrent_id);

			mysql_query(conn,query2);
			sprintf(error,"Error: %s",mysql_error(conn));
			res_set2 = mysql_store_result(conn);
			if (mysql_errno(conn) == 0)
			{
				string temphash;
				numrows = (unsigned int)mysql_num_rows(res_set2);

				BTIP tempIP;
				BTIPList templist;
				int num = 0;

				while ((row2 = mysql_fetch_row(res_set2)) != NULL) 
				{
					num ++;
					tempIP.m_torrent_id = atoi(row2[0]);
					strcpy(tempIP.m_ip,row2[1]);
					tempIP.m_port = atoi(row2[2]);
					strcpy(tempIP.m_id,row[3]);
					templist.v_ips.push_back(tempIP);
				}
				templist.m_num_ips = num;
				vips->push_back(templist);
				
			}

			mysql_free_result(res_set2);
/*
			sprintf(query2,"select piece_hash from Torrent_pieces where hash_id = %d Order by piece_number",tempdata.m_torrent_id);
			mysql_query(conn,query2);
			sprintf(error,"Error: %s",mysql_error(conn));
			res_set2 = mysql_store_result(conn);

			if (mysql_errno(conn) == 0)
			{
				string temphash;
				numrows = (unsigned int)mysql_num_rows(res_set2);

				FileInfo tempinfo;

				while ((row2 = mysql_fetch_row(res_set2)) != NULL) 
				{
					temphash = row2[0];
					tempdata.v_piece_hashes.push_back(temphash);

				}
			}

			mysql_free_result(res_set2);
			*/
			tfiles->push_back(tempdata);
		}
	}

	mysql_free_result(res_set);
	}
	mysql_close(conn);

}

void BTDatabaseInterface::GetIPListSubset(vector<HashIdPair> torrent_ids, vector<BTIPList> * vips)
{

	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set;
	MYSQL_ROW row;

	int i = 0;
	int numrows;
	TorrentFileData temp_entry;


	char query[1024+1];
	for (int i = 0; i< (int)torrent_ids.size(); i++)
	{

	//	sprintf(query,"select hash_id, ip, port from torrent_ips where hash_id = %d",torrent_ids[i].torrent_id);

		sprintf(query,"select torrent_ips.hash_id, torrent_ips.ip, torrent_ips.port, torrent_id.hash from torrent_ips, torrent_id where torrent_id.hash_id = %d and torrent_id.hash_id = torrent_ips.hash_id",torrent_ids[i].torrent_id);
		mysql_query(conn,query);
		sprintf(error,"Error: %s",mysql_error(conn));
		res_set = mysql_store_result(conn);
		if (mysql_errno(conn) == 0)
		{
			string temphash;
			numrows = (unsigned int)mysql_num_rows(res_set);

			BTIP tempIP;
			BTIPList templist;
			int num = 0;

			while ((row = mysql_fetch_row(res_set)) != NULL) 
			{
				num ++;
				tempIP.m_torrent_id = atoi(row[0]);
				strcpy(tempIP.m_ip,row[1]);
				tempIP.m_port = atoi(row[2]);
				strcpy(tempIP.m_id,row[3]);
				templist.v_ips.push_back(tempIP);
			}
			templist.m_num_ips = num;
			vips->push_back(templist);
				
		}

		mysql_free_result(res_set);
		mysql_close(conn);
	}
}


void BTDatabaseInterface::GetHashList(vector<HashIdPair> *hashpairs)
{

	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set;
	MYSQL_ROW row;

	int i = 0;
	int numrows;

	CString query ="Select hash_id, hash, time from torrent_id";
	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	if (mysql_errno(conn) == 0)
	{
		numrows = (unsigned int)mysql_num_rows(res_set);

		HashIdPair tempdata;

		while ((row = mysql_fetch_row(res_set)) != NULL) 
		{

			tempdata.torrent_id=atoi(row[0]);
			strcpy(tempdata.hash,row[1]);
			strcpy(tempdata.timestamp,row[2]);
			hashpairs->push_back(tempdata);
		}
	}

	mysql_free_result(res_set);
	mysql_close(conn);
}

/*
void BTDatabaseInterface::GetLowSeedTorrentInfo(vector<TorrentFileData> * tfiles)
{
	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	MYSQL_RES *res_set; 
	MYSQL_ROW row;

	int i = 0;
	int numrows;
	TorrentFileData temp_entry;

	
	CString query ="Select torrent_id.hash, announce_list.url, torrent_id.hash_id from torrent_id, announce_list, torrent_seed_count WHERE to_days(torrent_id.time) >= to_days(now())-1 AND torrent_id.hash_id = announce_list.hash_id AND torrent_seed_count.hash_id = torrent_id.hash_id AND torrent_seed_count.seed_count = 0";

	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	if (mysql_errno(conn) == 0)
	{
		numrows = (unsigned int)mysql_num_rows(res_set);

		TorrentFileData tempdata;

		while ((row = mysql_fetch_row(res_set)) != NULL) 
		{

			tempdata.m_hash=row[0];
			tempdata.m_announce_URL=row[1];
			tempdata.m_torrent_id = atoi(row[2]);
			tfiles->push_back(tempdata);
		}
	}

	mysql_free_result(res_set);
	mysql_close(conn);

}

*/

void BTDatabaseInterface::GetLowSeedTorrentInfo(vector<TorrentFileData> * tfiles)
{
	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0);
	sprintf(error,"Error: %s",mysql_error(conn));

	/*
	if (mysql_errno(conn)!= 0)
	{
		return;
	}
*/
	MYSQL_RES *res_set; 
	MYSQL_ROW row;

	int i = 0;
	int numrows;
	TorrentFileData temp_entry;

	
	CString query ="Select hash, url, hash_id, name from seed_inflation_data";
	//CString query ="Select hash, url, hash_id, name from seed_inflation_data limit 30";
	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));
	res_set = mysql_store_result(conn);
	sprintf(error,"Error: %s",mysql_error(conn));
	if (mysql_errno(conn) == 0)
	{
		numrows = (unsigned int)mysql_num_rows(res_set);

		TorrentFileData tempdata;

		while ((row = mysql_fetch_row(res_set)) != NULL) 
		{

			tempdata.m_hash=row[0];
			tempdata.m_announce_URL=row[1];
			tempdata.m_torrent_id = atoi(row[2]);
			tempdata.m_name = row[3];
			tfiles->push_back(tempdata);
		}
	}

	mysql_free_result(res_set);
	mysql_close(conn);

}

void BTDatabaseInterface::AddLowSeedTorrent(TorrentFileData tfile)
{
	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	if (mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0)== 0)
	{
		TRACE0("database connection Failed");
		return;
	}
	sprintf(error,"%s",mysql_error(conn));


	char query[1024+1];
	sprintf(query,"Insert into seed_inflation_data values(%d,'%s','%s','%s',0,'M','NULL')",tfile.m_torrent_id,tfile.m_name.c_str(),tfile.m_announce_URL.c_str(),tfile.m_hash.c_str());
	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));


	mysql_close(conn);

}


void BTDatabaseInterface::RemoveLowSeedTorrent(TorrentFileData tfile)
{

	char error[256];

	MYSQL *conn;
	conn = mysql_init(NULL);
	if (mysql_real_connect(conn,"206.161.141.35","onsystems","ebertsux37","bittorrent_data",0,NULL,0)== 0)
	{
		TRACE0("database connection Failed");
		return;

	}

	sprintf(error,"Error: %s",mysql_error(conn));

/*	
	if (mysql_errno(conn)!= 0)
	{
		return;
	}
*/
	char query[1024+1];
	sprintf(query,"Delete from seed_inflation_data where url = '%s' and hash = '%s'",tfile.m_announce_URL.c_str(),tfile.m_hash.c_str());
	mysql_query(conn,query);
	sprintf(error,"Error: %s",mysql_error(conn));


	mysql_close(conn);



}