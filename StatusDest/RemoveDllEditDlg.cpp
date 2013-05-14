// RemoveDllEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RemoveDllEditDlg.h"


// CRemoveDllEditDlg dialog

IMPLEMENT_DYNAMIC(CRemoveDllEditDlg, CDialog)
CRemoveDllEditDlg::CRemoveDllEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRemoveDllEditDlg::IDD, pParent)
	, m_filename(_T(""))
{
}

CRemoveDllEditDlg::~CRemoveDllEditDlg()
{
}

void CRemoveDllEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FILENAME_EDIT, m_filename);
}


BEGIN_MESSAGE_MAP(CRemoveDllEditDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CRemoveDllEditDlg message handlers

void CRemoveDllEditDlg::OnBnClickedOk()
{
	UpdateData();
	::SendMessage(m_parent_hwnd,WM_DONE_REMOVE_DLL_EDIT,(WPARAM)(const char*)m_filename, 0);
	OnOK();
}

//
//
//
void CRemoveDllEditDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if(bShow)
		GetDlgItem(IDC_FILENAME_EDIT)->SetFocus();
}
