#pragma once
#include "StdAfx.h"
#include "Torrent.h"
#include "TorrentIndexThreadData.h"
#include <afxmt.h>

// TorrentIndex

//
//
//
class CCriticalSection;
class CTorrentManager;
class TorrentIndexRequest;
class TorrentIndex
{
// functions
public:
	TorrentIndex(HWND parentHwnd);
	~TorrentIndex(void);

	void TorrentQuery(TorrentIndexRequest *pTorrentIndexRequest);

	void AddTorrent(Torrent *pTorrent);
	void RemoveTorrent(const Hash20Byte &rInfoHash);
	void KillStaleIPs(void);
	void GetConnectedPeers(const Hash20Byte *pHash);

// variables
private:
	HWND						m_managerHwnd;
	CCriticalSection			m_criticalSection;
	TorrentIndexThreadData  	m_threadData;
	CWinThread *				m_pThread;
};
