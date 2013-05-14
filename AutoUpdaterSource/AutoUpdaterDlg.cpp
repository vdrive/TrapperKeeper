// AutoUpdaterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutoUpdaterSource.h"
#include "AutoUpdaterDlg.h"


// CAutoUpdaterDlg dialog

IMPLEMENT_DYNAMIC(CAutoUpdaterDlg, CDialog)
CAutoUpdaterDlg::CAutoUpdaterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAutoUpdaterDlg::IDD, pParent)
{
}

CAutoUpdaterDlg::~CAutoUpdaterDlg()
{
}

void CAutoUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAutoUpdaterDlg, CDialog)
END_MESSAGE_MAP()


// CAutoUpdaterDlg message handlers
