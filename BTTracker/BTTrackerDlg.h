#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "AddInfohashDlg.h"
#include "ConnectedPeersDlg.h"

// CBTTrackerDlg dialog

class Peer;
class Torrent;
class BTTrackerDll;
class TorrentListUpdate;
class CBTTrackerDlg 
	: public CDialog
{
	DECLARE_DYNAMIC(CBTTrackerDlg)

public:
	CBTTrackerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBTTrackerDlg();

	inline void Init(BTTrackerDll *pDll) { m_pDll = pDll; }
	void UpdateTorrentList(TorrentListUpdate *pTorrentListUpdate);
	void LogEntry(const char *pStr);
	void AddTorrent(Torrent *pTorrent);
	void ConnectedPeers(const set<Peer> *pConnectedPeers);

	void SetCounts(size_t nMods, size_t nClients);

	// Dialog Data
	enum { IDD = IDD_BTTRACKER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnBnClickedAddinfohashButton();
	afx_msg void OnNMDblclkTorrentIndexList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedStaleCheckButton();

	DECLARE_MESSAGE_MAP()

private:
	BTTrackerDll *		m_pDll;

	CListCtrl			m_torrentLogList;
	CListCtrl			m_torrentIndexList;

	CAddInfohashDlg		m_addInfohashDlg;
	CConnectedPeersDlg	m_connectedPeersDlg;

	int					m_nMaxLogEntries;

	CStatic				m_clientsText;
	CStatic				m_modulesText;
};
