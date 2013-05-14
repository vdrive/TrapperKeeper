#pragma once
#include "Resource.h"

#define	WM_KAZAA_HWND			WM_USER+1

// CStatusSourceDlg dialog

class StatusSourceDll;
class CStatusSourceDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatusSourceDlg)

public:
	CStatusSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStatusSourceDlg();
	void InitParent(StatusSourceDll* parent);

// Dialog Data
	enum { IDD = IDD_STATUS_SOURCE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
	StatusSourceDll* p_parent;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnKazaaHwnd(WPARAM wparam,LPARAM lparam);
};
