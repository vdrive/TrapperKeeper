#pragma once
#include "Resource.h"

// CEditDlg dialog

class CEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditDlg)

public:
	CEditDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditDlg();

// Dialog Data
	enum { IDD = IDD_EDIT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	HWND m_parent_hwnd;
	afx_msg void OnBnClickedOk();
private:
	UINT m_max_kazaa;
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
