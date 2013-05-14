#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "afxcmn.h"


// CAddDecoyDlg dialog
class CAddDecoyDlg
	: public CDialog
{
	// Construction
public:
	CAddDecoyDlg(CWnd* pParent = NULL);	// standard constructor

	// Dialog Data
	enum { IDD = IDD_ADD_DECOY_DLG };

protected:
	void AutoSizeAdjust(void);

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual void OnOK() {}; 
	virtual void OnCancel() {}; 
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAddItemButton();
	afx_msg void OnBnClickedRemoveItemButton();
	afx_msg void OnBnClickedCommitButton();
	afx_msg void OnBnClickedClearButton();
	afx_msg void OnNMClickTorrentFileContentsList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedMultifileTorrentCheck();
	afx_msg void OnBnClickedForcePrimaryCheck();
	afx_msg void OnBnClickedRefreshTrackersButton();
	afx_msg void OnEnChangeItemFilesizeEdit();
	void OnTimer(UINT nIDEvent);
	afx_msg LRESULT GetTrackersResult(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT SubmitDataResult(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()

private:
	CEdit			m_torrentFilenameEdit;
	CStatic			m_totalFilesizeStatic;
	CStatic			m_remaininglFilesizeStatic;
	CListCtrl		m_torrentContentsList;
	CEdit			m_itemFilenameEdit;
	CEdit			m_itemFilesizeEdit;
	CStatic			m_commaItemFilesizeStatic;
	CEdit			m_baseDirectoryEdit;
	CButton			m_multifileCheck;
	CButton			m_commitButton;
	CButton			m_autoSizeAdjust;
	CComboBox		m_trackerCombo;
	CEdit			m_seedsBaseEdit;
	CEdit			m_seedsMultiplierEdit;
	CEdit			m_peersBaseEdit;
	CEdit			m_peersMultiplierEdit;
	CEdit			m_completeBaseEdit;
	CEdit			m_completeMultiplierEdit;
	CEdit			m_forcedTrackerEdit;
	CButton			m_forcedTrackerCheck;

	CWinThread *	m_pSubmitDataThread;

	char			m_aLocalIP[16];
};
