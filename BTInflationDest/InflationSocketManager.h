#pragma once
#include "inflationSocket.h"
#include "Torrent.h"

class BTInflatorDll;
class InflationSocketManager
{
public:
	InflationSocketManager(void);
	~InflationSocketManager(void);

	void InitParent(BTInflatorDll * parent);


	void MakeTorrentConnections(Torrent * the_torrent);
	void MakeNewTorrentConnections(Torrent * the_torrent);
	void TorrentFailure(string info_hash, string announce_url, string peer_id, InflationSocket * socketptr);
	void TorrentSuccess(string info_hash, string announce_url, string peer_id, InflationSocket * socketptr);

	void TorrentFailure(InflationSocket * socketptr);
	void TorrentSuccess(InflationSocket * socketptr);

	void ParseHttpString(char * input, char * host, char * url, int * port);
	void ClearStaleSockets();

private:

	BTInflatorDll * p_parent;
	vector<InflationSocket *> v_sockets;
	
};
