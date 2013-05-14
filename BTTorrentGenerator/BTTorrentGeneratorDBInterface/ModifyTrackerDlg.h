#pragma once
#include "afxcmn.h"
#include <vector>
#include "afxwin.h"
//#include "afxwin.h"
using namespace std;

// CModifyTrackerDlg dialog

class CModifyTrackerDlg : public CDialog
{
	DECLARE_DYNAMIC(CModifyTrackerDlg)

public:
	CModifyTrackerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CModifyTrackerDlg();

// Dialog Data
	enum { IDD = IDD_MODIFY_TRACKER_DLG };

protected:
	virtual void OnOK() {}; 
	virtual void OnCancel() {}; 
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAddTrackerButton();
	afx_msg void OnBnClickedSetActiveTrackerButton();
	afx_msg void OnBnClickedSetInactiveTrackerButton();
	afx_msg void OnNMClickTrackersList(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnTimer(UINT nIDEvent);
	afx_msg LRESULT GetTrackersResult(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()

protected:
	CListCtrl	m_trackersList;
	CEdit		m_trackerEdit;
public:
	afx_msg void OnLvnKeydownTrackersList(NMHDR *pNMHDR, LRESULT *pResult);
};
