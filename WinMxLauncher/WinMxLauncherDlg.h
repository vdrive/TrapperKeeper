#pragma once
#include "afxwin.h"
#include "Resource.h"


// CWinMxLauncherDlg dialog

class WinMxLauncherDll;
class CWinMxLauncherDlg : public CDialog
{
	DECLARE_DYNAMIC(CWinMxLauncherDlg)

public:
	CWinMxLauncherDlg(CWnd* pParent = NULL);   // standard constructor
	void InitParent(WinMxLauncherDll* parent);
	virtual ~CWinMxLauncherDlg();

// Dialog Data
	enum { IDD = IDD_WinMx_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_log_edit;
private:
	WinMxLauncherDll* p_parent;
public:
	afx_msg void OnTimer(UINT nIDEvent);
};
