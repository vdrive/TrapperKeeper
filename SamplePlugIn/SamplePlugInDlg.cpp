// SamplePlugInDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SamplePlugInDlg.h"
#include "DllInterface.h"


// CSamplePlugInDlg dialog

IMPLEMENT_DYNAMIC(CSamplePlugInDlg, CDialog)
CSamplePlugInDlg::CSamplePlugInDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSamplePlugInDlg::IDD, pParent)
{
}

CSamplePlugInDlg::~CSamplePlugInDlg()
{
}

void CSamplePlugInDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSamplePlugInDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_KILL_US_ALL, OnBnClickedKillUsAll)
END_MESSAGE_MAP()


// CSamplePlugInDlg message handlers
void CSamplePlugInDlg::OnCancel()		// user hit ESC
{
//	CDialog::OnCancel();
//	OnExit();
}

//
//
//
void CSamplePlugInDlg::OnClose()
{
	ShowWindow(SW_HIDE);	
}

//
//
//
void CSamplePlugInDlg::OnBnClickedKillUsAll()
{
	vector<AppID> apps;
	
	AppID app;
	app.m_app_id = 99999;
	app.m_version = 3;
	app.m_app_name = "test1";
	apps.push_back(app);
	
	DllInterface::ReloadDlls(apps);
}
