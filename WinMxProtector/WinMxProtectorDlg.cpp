// WinMxProtectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinMxProtectorDlg.h"
#include "resource.h"


// CWinMxProtectorDlg dialog

IMPLEMENT_DYNAMIC(CWinMxProtectorDlg, CDialog)
CWinMxProtectorDlg::CWinMxProtectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWinMxProtectorDlg::IDD, pParent)
{
}

CWinMxProtectorDlg::~CWinMxProtectorDlg()
{
}

void CWinMxProtectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CWinMxProtectorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_control.InsertColumn(0,"Output",LVCFMT_LEFT,1000);
	return true;
}


BEGIN_MESSAGE_MAP(CWinMxProtectorDlg, CDialog)
END_MESSAGE_MAP()


// CWinMxProtectorDlg message handlers
