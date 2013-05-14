#pragma once
#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"

// UsenetPosterDlg dialog

class UsenetPosterDll;

class UsenetPosterDlg : public CDialog
{
	DECLARE_DYNAMIC(UsenetPosterDlg)

public:
	UsenetPosterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~UsenetPosterDlg();

	void Log(char *buf);

	void InitParent(UsenetPosterDll* dll);
	UsenetPosterDll *p_dll;

// Dialog Data
	enum { IDD = IDD_USENET_POSTER_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_list_box;
	CListCtrl m_servers_list;
	afx_msg void OnTimer(UINT nIDEvent);
};
