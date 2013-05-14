// StatusSourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "StatusSourceDlg.h"
#include "StatusSourceDll.h"


// CStatusSourceDlg dialog

IMPLEMENT_DYNAMIC(CStatusSourceDlg, CDialog)
CStatusSourceDlg::CStatusSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatusSourceDlg::IDD, pParent)
{
}

CStatusSourceDlg::~CStatusSourceDlg()
{
}

void CStatusSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CStatusSourceDlg, CDialog)
	ON_WM_TIMER()
	ON_MESSAGE(WM_KAZAA_HWND, OnKazaaHwnd)
END_MESSAGE_MAP()


// CStatusSourceDlg message handlers
void CStatusSourceDlg::InitParent(StatusSourceDll* parent)
{
	p_parent = parent;
}

//
//
//
void CStatusSourceDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
afx_msg LRESULT CStatusSourceDlg::OnKazaaHwnd(WPARAM wparam,LPARAM lparam)
{
	p_parent->OnKazaaHwnd(wparam, lparam);
	return 0;
}
