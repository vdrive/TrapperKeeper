#pragma once

//#include "libtorrent\hasher.hpp"
//#include "sha1.cpp"
//#include "libtorrent\peer_id.hpp"
#include "stdafx.h"


class BTorrent
{
public:
	BTorrent(void);
	~BTorrent(void);

	struct torrent_hash
	{
		byte m_hash[20];
	};


private:

	int m_piece_length;
	vector<torrent_hash> m_piece_hash;
	vector<string> m_files;
	int m_total_size;
	vector<string>m_seeds;
	vector<string>m_trackers;

	// the hash that identifies this torrent
	torrent_hash m_info_hash;
	string m_name;


	//Unneded information stored in the torrent
	CTime m_creation_date;
	string m_comment;
	string m_created_by;
	
};
