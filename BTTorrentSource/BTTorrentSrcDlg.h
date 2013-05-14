#pragma once
#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "torrentfiledata.h"

// BTTorrentSrcDlg dialog

class BTTorrentSourceDll;
class BTTorrentSrcDlg : public CDialog
{
	DECLARE_DYNAMIC(BTTorrentSrcDlg)

public:
	BTTorrentSrcDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTTorrentSrcDlg();

	void InitParent(BTTorrentSourceDll* parent);

// Dialog Data
	enum { IDD = IDD_BT_TORRENT_SRC_DLG };


	void OnTimer(UINT nIDEvent);
	BOOL OnInitDialog();
	void RefreshList(vector<TorrentFileData> tdata);
	void RefreshList(vector<TorrentFileData> tdata, vector<TorrentFileData> tdata2);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	BTTorrentSourceDll * p_parent;
	int m_torrentid;

public:
	CButton m_add_torrent_button;
	afx_msg void OnBnClickedAddTorrent();
	CEdit m_announce_url_ctl;
	CEdit m_info_hash_ctl;
	afx_msg void OnBnClickedRemoveBtn();
	CListCtrl m_inflation_ctl;
	CEdit m_torrent_filename_ctl;
};
