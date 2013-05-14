#pragma once
#include "afxcmn.h"
#include <vector>
#include "afxwin.h"
using namespace std;

// CModifyStatsDlg dialog

class TorrentEntry;
class CModifyStatsDlg : public CDialog
{
	DECLARE_DYNAMIC(CModifyStatsDlg)

public:
	CModifyStatsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CModifyStatsDlg();

// Dialog Data
	enum { IDD = IDD_MODIFY_STATS_DLG };

protected:
	virtual void OnOK() {}; 
	virtual void OnCancel() {}; 
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT GetTorrentsResult(WPARAM wparam, LPARAM lparam);
	afx_msg void OnNMClickTorrentList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownTorrentList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedEditButton();
	afx_msg void OnBnClickedCommitButton();
	afx_msg void OnBnClickedCancelButton();
	afx_msg void OnBnClickedActiveCheck();
	afx_msg void OnBnClickedGenerateCheck();
	afx_msg void OnBnClickedInflateCheck();
	afx_msg void OnEnChangeCommentEdit();

	DECLARE_MESSAGE_MAP()

protected:
	CListCtrl					m_torrentsList;
	vector<TorrentEntry> *		m_pvTorrents;

	bool						m_bEditing;
	CTime						m_editingStartTime;
	int							m_nEditSelection;
};
