#pragma once

#include "torrentfile.h"
#include "afxsock.h"

class PeerConnector :
	public CAsyncSocket
{
	DECLARE_DYNAMIC(PeerConnector);

public:
	PeerConnector(void);
	PeerConnector(TorrentFile * tf);
	~PeerConnector(void);

	int FindSeeds();
	void OnSend(int errorCode);
	void OnAccept(int errorCode);
	void OnConnect(int errorCode);
	void OnClose(int errorCode);

protected:
	TorrentFile * m_torrent_file;
	int FindSeeds(PeerList *peers);
	void Connect(PeerList *peer);
	bool Handshake(PeerList * peer);
	bool IsSeed(PeerList *peer);
	char * GetHandshakeMessage(PeerList * peer);
	PeerList * m_peer;
};
