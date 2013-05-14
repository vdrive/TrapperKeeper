#pragma once
#include "afxwin.h"


// CHashIdentifierDlg dialog

class CHashIdentifierDlg : public CDialog
{
	DECLARE_DYNAMIC(CHashIdentifierDlg)

public:
	CHashIdentifierDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHashIdentifierDlg();

// Dialog Data
	enum { IDD = IDD_HASH_IDENTIFIER_DLG };

protected:
	CString GetCurrentPath();
	void PopulateFileList(CString path);

	virtual void OnOK() {}; 
	virtual void OnCancel() {}; 
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnNMDblclkFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckTorrentsButton();
	afx_msg void OnLvnKeydownDirectoryList(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg LRESULT OnIncrementReal(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnIncrementDecoy(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnIncrementInflated(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnIncrementCrossnamed(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnCheckTorrentsThreadDone(WPARAM wparam, LPARAM lparam);

	afx_msg LRESULT OnProgress(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnStatus(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()

protected:
	CListCtrl		m_torrentFilesList;
	CProgressCtrl	m_checkingProg;
	CImageList		m_iconImagesList;

	int				m_nReal;
	int				m_nDecoy;
	int				m_nInflated;
	int				m_nCrossnamed;
};
