// AutoUpdaterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutoUpdaterDlg.h"
#include "AutoUpdaterDll.h"

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
	DDX_Control(pDX, IDC_LIST1, m_list_box);
}


BEGIN_MESSAGE_MAP(CAutoUpdaterDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RELOAD_BUTTON, OnBnClickedReloadButton)
	ON_BN_CLICKED(IDC_SUCCESS_BUTTON, OnBnClickedSuccessButton)
	ON_BN_CLICKED(IDC_GOODZ, OnBnClickedGoodz)
END_MESSAGE_MAP()


// CAutoUpdaterDlg message handlers
void CAutoUpdaterDlg::OnCancel()		// user hit ESC
{
//	CDialog::OnCancel();
//	OnExit();
}

//
//
//
void CAutoUpdaterDlg::OnClose()
{
	ShowWindow(SW_HIDE);	
}

void CAutoUpdaterDlg::OnBnClickedReloadButton()
{
	//
	//	Post the WM_RELOAD message to the auto-updater watcher executable
	//
	UINT WM_RELOAD = RegisterWindowMessage("RELOAD");

	if (WM_RELOAD != 0)
		::PostMessage(HWND_BROADCAST,WM_RELOAD,(WPARAM)0,(LPARAM)0);
	else
		MessageBox("RegisterWindowMessage Error", "We got a return of 0, this is not good.", MB_OK);
}

void CAutoUpdaterDlg::OnBnClickedSuccessButton()
{
	//
	//	Post the WM_SUCCESS message to the auto-updater watcher executable
	//
	UINT WM_SUCCESS = RegisterWindowMessage("SUCCESS");

	if (WM_SUCCESS != 0)
		::PostMessage(HWND_BROADCAST,WM_SUCCESS,(WPARAM)0,(LPARAM)0);
	else
		MessageBox("RegisterWindowMessage Error", "We got a return of 0, this is not good.", MB_OK);
}


void CAutoUpdaterDlg::InitAutoDll(AutoUpdaterDll *dll)
{
	p_dll=dll;
}


//
//	Send a message to the AutoUpdaterSource and request the list of good DLL names
//
void CAutoUpdaterDlg::OnBnClickedGoodz()
{
	// for testing
	p_dll->GetGoodDllsFromSource("38.118.139.68");
}
