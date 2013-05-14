#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "Torrent.h"
#include <pdh.h>						// also must include pdh.lib
#include <set>
using namespace std;

class Hash20Byte;
class BTDecoyClientDll;
class CBTDecoyClientDlg
	: public CDialog
{
	DECLARE_DYNAMIC(CBTDecoyClientDlg)
	enum { IDD = IDD_DECOYCLIENT_DIALOG };

public:
	CBTDecoyClientDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBTDecoyClientDlg();

	inline void InitParent(BTDecoyClientDll *pDll) { m_pDll = pDll; }

	void UpdateTorrents(const map<Hash20Byte, const Torrent *> *pTorrents);

	inline void SetCompletedCheck(void) { m_completeCheck.SetCheck( BST_CHECKED ); }
	inline void SetCompletedUncheck(void) { m_completeCheck.SetCheck( BST_UNCHECKED ); }

	void InitPerformanceCounters(void);
	inline unsigned int GetMemLeft(void) const { return m_nMemLeft; }

	void LogMsg(const char *pStr);

private:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnHdnItemdblclickTorrentsList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

private:
	BTDecoyClientDll *	m_pDll;

	CListCtrl			m_torrentsList;
	CListCtrl			m_logList;
	int					m_nMaxLogEntries;
	CButton				m_completeCheck;

	CStatic				m_modulesStatic;
	CStatic				m_clientsStatic;

	HQUERY				m_pdh;
	HCOUNTER			m_pdh_counters[3];

	unsigned int		m_nMemLeft;
	unsigned int		m_nMemUse;

	vector<string>								m_vTorrentFilename;
	vector<string>								m_vTorrentHash;
	vector<string>								m_vTorrentTotalLength;
	vector<string>								m_vTorrentID;
	vector< map<unsigned int, unsigned int> >	m_vmTorrentPieces;
	vector<unsigned int>						m_vTorrentTotalPieces;
	vector<CTime>								m_vTorrentActivationTime;
};
