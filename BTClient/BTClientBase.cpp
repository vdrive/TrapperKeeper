#include "StdAfx.h"
#include "btclientbase.h"

// OBSOLETE do not use this class as is


BTClientBase::BTClientBase(void)
{
}

BTClientBase::~BTClientBase(void)
{
}

void BTClientBase::SpawnProjectSession(TorrentFile torrent)
{

	// Delete this when you close the connections
	CClientConnection *connection = new CClientConnection;
	connection->StartConnections(torrent);
	v_connections.push_back(connection);

}

void BTClientBase::DataReceived(char *source_ip, void *data, UINT data_length)
{

	TorrentFile thetorrent(data,data_length);
	SpawnProjectSession(thetorrent);

}
