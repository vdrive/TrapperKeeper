#pragma once
#include "Resource.h"
#include "afxwin.h"

// CAutoUpdaterDlg dialog

class AutoUpdaterDll;

class CAutoUpdaterDlg : public CDialog
{
	DECLARE_DYNAMIC(CAutoUpdaterDlg)

public:
	CAutoUpdaterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAutoUpdaterDlg();
	void OnCancel();

	void InitAutoDll(AutoUpdaterDll *dll);
	AutoUpdaterDll *p_dll;

// Dialog Data
	enum { IDD = IDD_AutoUpdater_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	CListBox m_list_box;
	afx_msg void OnBnClickedReloadButton();
	afx_msg void OnBnClickedSuccessButton();
	afx_msg void OnBnClickedGoodz();
};
