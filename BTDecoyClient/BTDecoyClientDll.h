
#pragma once
#include "Dll.h"
#include "../DllLoader/AppID.h"
#include "BTDecoyClientDlg.h"
#include "ClientReceiver.h"
#include "TorrentManager.h"
#include "DatabaseManager.h"
#include "ConnectionManager.h"

//
//
//
class Torrent;
class TorrentSearchRequest;
class TorrentSearchResult;
class AcceptedSocket;
class BTDecoyClientDll
	: public Dll
{
public:
	BTDecoyClientDll(void);
	~BTDecoyClientDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	inline void DllVerInit(unsigned int nVer) { m_databaseManager.InitVersion( nVer ); }

	void AddNewClient(const AcceptedSocket *pSocket);

	void TorrentSearch(TorrentSearchRequest *pTorrentSearchRequest);
	void TorrentSearch(TorrentSearchResult *pTorrentSearchResult);

	void ReceivedNewTorrents(map<Hash20Byte, const Torrent *> *pmpTorrents);

	inline size_t GetModuleCount(void) { return m_connectionManager.ReturnModCount(); }
	inline size_t GetClientCount(void) { return m_connectionManager.ClientCountsReq(); }

	void LogMsg(const char *pDataTemplate, ...);

private:
	CBTDecoyClientDlg		m_dlg;

	CClientReceiver			m_clientReceiver;
	CTorrentManager			m_torrentManager;
	CDatabaseManager		m_databaseManager;
	ConnectionManager		m_connectionManager;

	unsigned char *			m_pData;
	int						m_nDataSize;

	bool					m_bCompleteClient;
};
