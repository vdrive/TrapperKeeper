// BTPosterDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "BTPoster.h"
#include "BTPosterDlg.h"


// BTPosterDlg dialog

IMPLEMENT_DYNAMIC(BTPosterDlg, CDialog)
BTPosterDlg::BTPosterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BTPosterDlg::IDD, pParent)
{
}

BTPosterDlg::~BTPosterDlg()
{
}

void BTPosterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(BTPosterDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnCbnSelchangeCombo2)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// BTPosterDlg message handlers


void BTPosterDlg::OnCbnSelchangeCombo2()
{
	// TODO: Add your control notification handler code here
}

void BTPosterDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
	TRACE("OK\n");
}
