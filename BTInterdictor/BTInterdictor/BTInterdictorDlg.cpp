// BTinterdictorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTinterdictorDlg.h"
#include "DllInterface.h"
#include "AppID.h"


// CBTinterdictorDlg dialog

IMPLEMENT_DYNAMIC(CBTinterdictorDlg, CDialog)
CBTinterdictorDlg::CBTinterdictorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBTinterdictorDlg::IDD, pParent)
{
}

CBTinterdictorDlg::~CBTinterdictorDlg()
{
}

void CBTinterdictorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBTinterdictorDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_KILL_US_ALL, OnBnClickedKillUsAll)
END_MESSAGE_MAP()


// CBTinterdictorDlg message handlers
void CBTinterdictorDlg::OnCancel()		// user hit ESC
{
//	CDialog::OnCancel();
//	OnExit();
}

//
//
//
void CBTinterdictorDlg::OnClose()
{
	ShowWindow(SW_HIDE);	
}

//
//
//
void CBTinterdictorDlg::OnBnClickedKillUsAll()
{

	vector<AppID> apps;
	
	AppID app;
	app.m_app_id = 99999;
	app.m_version = 3;
	app.m_app_name = "test1";
	apps.push_back(app);
	
	DllInterface::ReloadDlls(apps);

}
