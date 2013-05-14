// EditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditDlg.h"

// CEditDlg dialog

IMPLEMENT_DYNAMIC(CEditDlg, CDialog)
CEditDlg::CEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditDlg::IDD, pParent)
	, m_max_kazaa(0)
{
}

CEditDlg::~CEditDlg()
{
}

void CEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT, m_max_kazaa);
}


BEGIN_MESSAGE_MAP(CEditDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CEditDlg message handlers

void CEditDlg::OnBnClickedOk()
{
	UpdateData();
	::SendMessage(m_parent_hwnd,WM_DONE_EDIT,(WPARAM)m_max_kazaa, 0);	
	OnOK();
}

//
//
//
void CEditDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if(bShow)
		GetDlgItem(IDC_EDIT)->SetFocus();
}
