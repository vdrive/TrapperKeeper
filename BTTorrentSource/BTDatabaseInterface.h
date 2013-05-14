#pragma once
#include "TorrentFileData.h"
#include "BTIPList.h"
#include "BTIPDataHeader.h"

#define BTDATABASEIP 206.161.141.35

class BTDatabaseInterface
{
public:
	BTDatabaseInterface(void);
	~BTDatabaseInterface(void);

	void GetIPInfo(vector<TorrentFileData> * tfiles);
	void InsertIPs(BTIPList iplist);
	void GetFullTorrentInfo(vector<TorrentFileData> * tfiles);
	void GetSingleTorrentInfo(TorrentFileData * tfile,int torrent_id);
	void GetTorrentInfoSubset(vector<TorrentFileData> * tfiles, vector<HashIdPair> torrent_ids, vector<BTIPList> * vips);
	void GetIPListSubset(vector<HashIdPair> torrent_ids, vector<BTIPList> * vips);
	void GetHashList(vector<HashIdPair> * hashpairs);
	void GetLowSeedTorrentInfo(vector<TorrentFileData> * tfiles);

	void AddLowSeedTorrent(TorrentFileData tfile);
	void RemoveLowSeedTorrent(TorrentFileData tfile);
};
