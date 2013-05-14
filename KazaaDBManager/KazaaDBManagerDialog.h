#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "KazaaDBSystem.h"


// KazaaDBManagerDialog dialog

class KazaaDBManagerDialog : public CDialog
{
	DECLARE_DYNAMIC(KazaaDBManagerDialog)

public:
	KazaaDBManagerDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~KazaaDBManagerDialog();

// Dialog Data
	enum { IDD = IDD_KAZAADBMANAGERDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	int m_last_progress;
	int m_last_max_progress;
	string m_status;
	KazaaDBSystem m_system;
public:
	afx_msg void OnScanFiles();
	afx_msg void OnWriteToDatabases();
	afx_msg void OnClearFiles();
	afx_msg void OnDeleteDatabases();
	CProgressCtrl m_progress_bar;
	CStatic m_status_text;
	CListCtrl m_file_list;
	CStatic m_seconds_to_rescan;
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	CStatic m_files_changing;
	CStatic m_kazaa_db_current;
	afx_msg void OnLvnColumnClickFilelist(NMHDR *pNMHDR, LRESULT *pResult);
	void StopSystem(void);
};
