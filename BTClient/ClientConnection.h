#pragma once

#include "ClientSocket.h"
#include "..\tkcom\threadedobject.h"
#include <afxmt.h>
#include "..\BTScraperDll\PeerList.h"
#include "..\BTScraperDll\TorrentFile.h"
#include "ListeningSocket.h"

class CClientConnection : protected ThreadedObject
{
public:
	CClientConnection(void);
	~CClientConnection(void);

	UINT Run();
	void StartConnections(TorrentFile& the_torrent);
	void Stop(void);

	
	struct PeerIpPort
	{
		string m_ip;
		string m_port;
	};
	
	char * CreatePeerID();
	unsigned char * GetHandshakeMessage();

private:

	
	CCriticalSection m_lock;
	vector<PeerIpPort> m_peerips;
	vector<CClientSocket*> mv_sockets;
	ListeningSocket m_listener;
	TorrentFile m_torrent;
	PeerList * m_peers;
	bool * p_bitfield;	
	char m_peer_id[20+1];

	int nextport;

};
