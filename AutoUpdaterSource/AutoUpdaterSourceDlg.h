#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "afxcmn.h"

// CAutoUpdaterSourceDlg dialog

class CAutoUpdaterSourceDlg : public CDialog
{
	DECLARE_DYNAMIC(CAutoUpdaterSourceDlg)

public:
	CAutoUpdaterSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAutoUpdaterSourceDlg();

// Dialog Data
	enum { IDD = IDD_AutoUpdaterSourceDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	
	CListBox m_list_current_plugins;
	CListCtrl m_list_update_log;
};
