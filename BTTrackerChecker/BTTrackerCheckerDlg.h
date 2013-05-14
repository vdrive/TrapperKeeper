#pragma once
#include "Resource.h"

// BTTrackerCheckerDlg dialog

class BTTrackerCheckerDll;

class BTTrackerCheckerDlg : public CDialog
{
	DECLARE_DYNAMIC(BTTrackerCheckerDlg)

public:
	BTTrackerCheckerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTTrackerCheckerDlg();

	void Log(string buf);
	void Log(const char *buf);
	
	// Dialog Data
	enum { IDD = IDD_BTTRACKERCHECKERDLG };

	void OnTimer(UINT nIDEvent);
	BTTrackerCheckerDll *p_dll;

protected:
	bool b_init;
	CListCtrl  m_list_ctrl;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
