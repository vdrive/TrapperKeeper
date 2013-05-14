#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "ConnectionModuleStatusData.h"



// COvernetSpooferMainDlg dialog
class OvernetSpooferDll;
class COvernetSpooferMainDlg : public CDialog
{
	DECLARE_DYNAMIC(COvernetSpooferMainDlg)

public:
	COvernetSpooferMainDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COvernetSpooferMainDlg();
	void InitParent(OvernetSpooferDll* parent);
	void Log(const char *buf);
	void StatusReady(ConnectionModuleStatusData& status);

// Dialog Data
	enum { IDD = IDD_OVERNET_SPOOFER_MAIN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
protected:
	CListBox m_log_list;
	OvernetSpooferDll* p_parent;
public:
	afx_msg void OnBnClickedResetLogButton();
};
