#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "ClientConnection.h"
#include "..\BTScraperDll\PeerList.h"
#include "..\BTScraperDll\TorrentFile.h"
#include "..\BTScraperDll\TorrentFileHeader.h"
#include "ConnectionManager.h"


// OBSOLETE do not use this class as is


class BTClientBase :
	public Dll
{
public:
	BTClientBase(void);
	~BTClientBase(void);

	void SpawnProjectSession(TorrentFile torrent);
	void DataReceived(char *source_ip, void *data, UINT data_length);

private:
	vector<CClientConnection*> v_connections;

};
