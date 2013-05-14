#pragma once
#include "afxcmn.h"
#include "resource.h"
#include "torrent.h"


// BTInflationDlg dialog
class BTInflatorDll;
class BTInflationDlg : public CDialog
{
	DECLARE_DYNAMIC(BTInflationDlg)

public:
	BTInflationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTInflationDlg();
	void OnTimer(UINT nIDEvent);
	void InitParent(BTInflatorDll * parent);
	void UpdatePeerData(string info_hash, string announce_url, PeerInfo info);
	void UpdateTorrentData(Torrent tor);
	void RefreshData();

// Dialog Data
	enum { IDD = IDD_Inflation_DLG };
	CTreeCtrl m_inflation_tree_ctl;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:

	BTInflatorDll * p_parent;
};
