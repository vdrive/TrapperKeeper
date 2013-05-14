// AutoUpdaterSourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutoUpdaterSourceDlg.h"


// CAutoUpdaterSourceDlg dialog

IMPLEMENT_DYNAMIC(CAutoUpdaterSourceDlg, CDialog)
CAutoUpdaterSourceDlg::CAutoUpdaterSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAutoUpdaterSourceDlg::IDD, pParent)
{
}

CAutoUpdaterSourceDlg::~CAutoUpdaterSourceDlg()
{
}

void CAutoUpdaterSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CURRENT_PLUGINS, m_list_current_plugins);
	DDX_Control(pDX, IDC_LIST1, m_list_update_log);
}


BEGIN_MESSAGE_MAP(CAutoUpdaterSourceDlg, CDialog)
END_MESSAGE_MAP()


// CAutoUpdaterSourceDlg message handlers
