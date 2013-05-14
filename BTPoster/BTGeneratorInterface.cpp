#include "StdAfx.h"
#include "btgeneratorinterface.h"
#include "SQLInterface.h"

BTGeneratorInterface::BTGeneratorInterface(void)
{
}

BTGeneratorInterface::~BTGeneratorInterface(void)
{
}


string BTGeneratorInterface::GenerateTorrentFile(string filename, string name, string size) 
{
	SQLInterface sql;
	MYSQL conn;
	MYSQL_RES * res;
	MYSQL_ROW row;

	string query;
	char buf[1024];

	string dir = name;
	string piece_len = "262114";
	string total_len = size;
	string id = "-1";

	if(!sql.OpenDB(&conn, "206.161.141.35", "bittorrent_data")) {
		return id;	
	}

	sprintf(buf, "insert into bt_generator_maininfo \
			(torrentfilename, topmost_file_or_dir, piece_length, \
			total_length, generate, active) \
			values('%s', '%s', %s, %s, 'f', 'f')",
			filename.c_str(), dir.c_str(), piece_len.c_str(), total_len.c_str());

	query = string(buf);
	if(!sql.Execute(&conn, query)) {
		return id;
	}

	sprintf(buf, "select max(auto_torrent_id) from bt_generator_maininfo \
				 where torrentfilename = '%s' and total_length = %s and \
				 topmost_file_or_dir = '%s' and piece_length = %s", 
				 filename.c_str(), total_len.c_str(), dir.c_str(), piece_len.c_str());
	query = string(buf);

	res = sql.Query(&conn, query);

	if(res != NULL)
	{
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			row = mysql_fetch_row(res);
			id = string(row[0]);
		}
		mysql_free_result(res);			// free's mem (::)
	}

	// TODO: non-default values for:	base_seeds,seed_inflation_multiplier,base_peers,
	// peer_inflation_multiplier,base_complete,complete_inflation_multiplier
	
	sprintf(buf, "insert into bt_generator_inflation (torrent_id) values(%s)", id.c_str());
	query = string(buf);
	sql.Execute(&conn, query);

	sprintf(buf, "insert into bt_generator_datafile \
				 (file_number, file_length, file_name, path, torrent_id) \
				 values (1, %s, '%s', '%s', %s)",
				 total_len.c_str(), filename.c_str(), dir.c_str(), id.c_str());
	query = string(buf);
	sql.Execute(&conn, query);


	query = string("select tracker_url from bt_generator_trackers where active = 'T' and type = 'url'");
	res = sql.Query(&conn, query);
	string tracker;
	if(res != NULL)
	{
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			row = mysql_fetch_row(res);
			tracker = string(row[0]);
		}
		mysql_free_result(res);			// free's mem (::)
	}

	sprintf(buf, "insert into bt_generator_announcelist \
				 (torrent_id, primary_tracker_url) \
				 values (%s, '%s')", id.c_str(), tracker.c_str());
	query = string(buf);
	sql.Execute(&conn, query);

	sprintf(buf, "update bt_generator_maininfo set generate ='t', active = 't' \
				 where auto_torrent_id = %s", id.c_str());
	query = string(buf);
	sql.Execute(&conn, query);

	sql.CloseDB(&conn);
	return id;	
}

bool BTGeneratorInterface::GetInfo(string id, string * p_filename, string * p_name, int * p_size)
{
	SQLInterface sql;
	MYSQL conn;
	MYSQL_RES * res;
	int size = -1;

	string query = "select torrent_size from bt_generator_torrentfile where torrent_id="+id;
	sql.OpenDB(&conn, "206.161.141.35", "bittorrent_data");

	res = sql.Query(&conn, query);
	if(res != NULL)
	{
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			size = atoi(row[0]);
			*p_size = size;
		}
		mysql_free_result(res);			// free's mem (::)
	}

	query = "select torrentfilename, topmost_file_or_dir from bt_generator_maininfo where auto_torrent_id = "+id;
	res = sql.Query(&conn, query);
	if(res != NULL)
	{
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			*p_filename = string(row[0]);
			*p_name = string(row[1]);
		}
		mysql_free_result(res);			// free's mem (::)
	}

	sql.CloseDB(&conn);
	return true;
}

bool BTGeneratorInterface::IsReady(string id) 
{
	SQLInterface sql;
	MYSQL conn;
	MYSQL_RES * res;
	bool b_ready = false;

	string query = "select * from bt_generator_maininfo \
				   where activation_date <> '19800101000000' and auto_torrent_id = "+ id;

	sql.OpenDB(&conn, "206.161.141.35", "bittorrent_data");
	res = sql.Query(&conn, query);
	
	if(res != NULL)
	{
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			b_ready = true;
		}
		mysql_free_result(res);			// free's mem (::)
	}
	sql.CloseDB(&conn);
	return b_ready;
}

int BTGeneratorInterface::GetTorrentFile(string id, char * torrent) 
{
	char buf[1024];
	SQLInterface sql;
	MYSQL conn;
	MYSQL_RES * res;
	MYSQL_ROW row;
	int size = -1;

	string query;

	sql.OpenDB(&conn, "206.161.141.35", "bittorrent_data");

	sprintf(buf, "select torrent_file, torrent_size from bt_generator_torrentfile where torrent_id = %s",
		id.c_str());
	
	query = string(buf);

	res = sql.Query(&conn, query);
	
	if(res != NULL)
	{
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			row = mysql_fetch_row(res);
			size = atoi(row[1]);
			memcpy(torrent, (row[0]), size);
		}
		mysql_free_result(res);			// free's mem (::)
	}
	sql.CloseDB(&conn);
	return size;

}