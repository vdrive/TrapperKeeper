#pragma once

#include "TorrentFileData.h"
#include "PeerList.h"


static char * chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

class TrackerConnection
{
public:

	TrackerConnection(void);
	~TrackerConnection(void);

	bool GetPeers(TorrentFileData * tf, string title);
	char * CreatePeerId();
	int GetData(const char * url, unsigned short **body, string referer);
	int GetHTTPData(const char * url, unsigned short **body, string referer,bool b_post, string post_data);
	int PostData(const char * url, unsigned short **body, string referer, string post_info);
	int UnZip(unsigned short ** dest, unsigned short * source, int len);
	char * GetTrackerURL(TorrentFileData * tf, const char *url, char * peer_id);
	char * GetKeyID();

	string ConvertToHash(string hash);



	
};
