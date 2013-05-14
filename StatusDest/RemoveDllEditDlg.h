#pragma once
#include "Resource.h"

// CRemoveDllEditDlg dialog

class CRemoveDllEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CRemoveDllEditDlg)

public:
	CRemoveDllEditDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRemoveDllEditDlg();

// Dialog Data
	enum { IDD = IDD_REMOVE_DLL_EDIT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString m_filename;
public:
	afx_msg void OnBnClickedOk();
	HWND m_parent_hwnd;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
