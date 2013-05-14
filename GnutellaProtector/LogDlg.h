#pragma once
#include "afxcmn.h"
#include "Resource.h"


// CLogDlg dialog

class CLogDlg : public CDialog
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogDlg();
	void OnCancel();
	void Log(char *text,COLORREF color=0,bool bold=false,bool italic=false);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	bool m_newline;

// Dialog Data
	enum { IDD = IDD_Log_Dialog };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl m_edit;
};
