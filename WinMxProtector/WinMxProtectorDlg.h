#pragma once
#include "resource.h"


// CWinMxProtectorDlg dialog
class WinMxProtectorDll;
class CWinMxProtectorDlg : public CDialog
{
	DECLARE_DYNAMIC(CWinMxProtectorDlg)

public:
	CWinMxProtectorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWinMxProtectorDlg();
	void InitParent(WinMxProtectorDll * p_parent);

	BOOL OnInitDialog();
	CListCtrl m_list_control;

// Dialog Data
	enum { IDD = IDD_WINMX_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	WinMxProtectorDll * p_parent;
};
