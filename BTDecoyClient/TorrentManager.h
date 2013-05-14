#pragma once
#include "Torrent.h"

#define WM_TMANAGER_INIT_THREAD_DATA			WM_USER+101
#define WM_TMANAGER_TORRENT_SEARCH				WM_USER+102
#define WM_TMANAGER_LOG_MSG						WM_USER+103

#define TMANAGER_EVENTS							3

// CTorrentManager

class BTDecoyClientDll;
class TorrentSearchRequest;
class TorrentSearchResult;
class TorrentManagerThreadData;
class CTorrentManager
	: private CWnd
{
public:
	CTorrentManager();
	virtual ~CTorrentManager();

	void InitParent(BTDecoyClientDll *pParent) { m_pParent = pParent; }
	void Run(void);
	void Stop(void);

	void ReceivedNewTorrents(map<Hash20Byte, const Torrent *> *pmpTorrents);
	void TorrentSearch(TorrentSearchRequest *pTorrentSearchRequest);

private:
	afx_msg LRESULT InitThreadData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT TorrentSearch(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT LogMsg(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

private:
	BTDecoyClientDll *				m_pParent;
	CCriticalSection *				m_pCriticalSection;
	TorrentManagerThreadData *		m_pThreadData;
	CWinThread *					m_pThread;
};
