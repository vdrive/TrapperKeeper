// ParentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SoulSeekServerDll.h"
#include "ParentDlg.h"


// ParentDlg dialog

IMPLEMENT_DYNAMIC(CParentDlg, CDialog)
CParentDlg::CParentDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CParentDlg::IDD, pParent)
{
}

CParentDlg::~CParentDlg()
{
}

void CParentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CParentDlg, CDialog)
END_MESSAGE_MAP()


// ParentDlg message handlers
