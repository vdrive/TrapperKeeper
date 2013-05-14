// ConnectionModuleThreadData.h
#pragma once
#include "winsock2.h"
#include "..\BTScraperDll\TorrentFile.h"

#include <vector>
using namespace std;


class ConnectionModuleStatusData;
/*
class SupernodeHost;
class VendorCount;
class FileSharingManager;
*/
class ConnectionModuleThreadData
{
public:
	ConnectionModuleThreadData();
	~ConnectionModuleThreadData(){};

	WSAEVENT m_reserved_events[4];
	ConnectionModuleStatusData *p_status_data;
//	vector<SupernodeHost> *p_connect_hosts;
	vector<SOCKET> *p_accepted_sockets;
	bool m_reconnect_to_hosts;
	bool m_disconnect_to_hosts;
	//FileSharingManager* p_file_sharing_manager;

	TorrentFile m_torrent;
	int m_port;

};