#pragma once


#define BTIPDATA_TORRENT 1
#define BTIPDATA_IP_LIST 2
#define BTIPDATA_HASH_LIST 3
#define BTIPDATA_TORRENT_REQUEST 4
#define BTIPDATA_TORRENT_LIST 5
#define BTIPDATA_IP_REQUEST 6 
#define BTIPDATA_LOW_SEED_TORRENT 7
#define BTIPDATA_STOP_SINGLE_INFLATION 8
#define BTIPDATA_STOP_ALL_INFLATION 9
#define BTIPDATA_PING 10
#define BTIPDATA_PONG 11


struct HashIdPair
{
	int torrent_id;
	char hash[40+1];
	char timestamp[14+1];
};

class BTIPDataHeader
{
public:
	BTIPDataHeader(void);
	~BTIPDataHeader(void);

	int appcode;
	int length;
};
