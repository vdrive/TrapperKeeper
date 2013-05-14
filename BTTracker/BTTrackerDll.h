#pragma once
#include "dll.h"
#include "..\DllLoader\AppID.h"
#include "BTTrackerDlg.h"
#include "ClientReceiver.h"
#include "ConnectionManager.h"
#include "TorrentManager.h"

class Torrent;
class Hash20Byte;
class TorrentListUpdate;
class TorrentIndexRequest;
class TorrentIndexResult;
class BTTrackerDll
	: public Dll
{
public:
	BTTrackerDll(void);
	~BTTrackerDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	void AddNewClient(SOCKET hSocket);
	void AddTorrent(Torrent *pTorrent);
	void GetConnectedPeers(const Hash20Byte *pHash);
	void ConnectedPeers(const set<Peer> *pPeers);

	void TorrentIndexReq(TorrentIndexRequest *pTorrentIndexRequest);
	void TorrentIndexRes(TorrentIndexResult *pTorrentIndexResponse);

	void CLogEntry(const char *pDataTemplate, ...);
	void UpdateTorrentList(TorrentListUpdate *pTorrentListUpdate);

	void CheckForStaleIPs(void);

private:
	CBTTrackerDlg		m_dlg;
	CClientReceiver		m_clientReceiver;
	ConnectionManager	m_clientManager;
	CTorrentManager		m_torrentManager;

public:
	int					m_nConnectedClients;
};
