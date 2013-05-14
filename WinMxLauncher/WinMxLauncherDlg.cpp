// WinMxLauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinMxLauncherDll.h"
#include "WinMxLauncherDlg.h"


// CWinMxLauncherDlg dialog

IMPLEMENT_DYNAMIC(CWinMxLauncherDlg, CDialog)
CWinMxLauncherDlg::CWinMxLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWinMxLauncherDlg::IDD, pParent)
{
}

CWinMxLauncherDlg::~CWinMxLauncherDlg()
{
}

void CWinMxLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_log_edit);
}


BEGIN_MESSAGE_MAP(CWinMxLauncherDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CWinMxLauncherDlg message handlers
void CWinMxLauncherDlg::InitParent(WinMxLauncherDll* parent)
{
	p_parent = parent;
}

//
//
//
void CWinMxLauncherDlg::OnTimer(UINT nIDEvent)
{
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}
