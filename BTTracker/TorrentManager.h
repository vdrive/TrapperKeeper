#pragma once
#include <afxmt.h>

// CTorrentManager

//
//
//
class Torrent;
class Hash20Byte;
class BTTrackerDll;
class TorrentIndexRequest;
class TorrentManagerThreadData;
class CTorrentManager
	: private CWnd
{
	DECLARE_DYNAMIC(CTorrentManager)

// functions
public:
	CTorrentManager();
	~CTorrentManager();

private:
	afx_msg LRESULT InitThreadData(WPARAM wparam, LPARAM lparam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

public:
	void Init(BTTrackerDll *pParent);
	void Run(void);
	void Stop(void);

	void TorrentIndexReq(TorrentIndexRequest *pTorrentIndexRequest);
	void AddTorrent(Torrent *pTorrent);

	void CheckForStaleIPs(void);
	void GetConnectedPeers(const Hash20Byte *pHash);

private:
	afx_msg LRESULT TorrentIndexRes(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT GetConnectedPeers(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT LogMsg(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT UpdateTorrentList(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

// variables
private:
	BTTrackerDll *					m_pParent;
	CWinThread *					m_pThread;
	CCriticalSection *				m_pCriticalSection;
	TorrentManagerThreadData *		m_pThreadData;

	UINT_PTR						m_StaleIPTimer;
};
