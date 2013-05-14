// MediaSwarmerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MediaSwarmerDest.h"
#include "MediaSwarmerDlg.h"


// MediaSwarmerDlg dialog

IMPLEMENT_DYNAMIC(MediaSwarmerDlg, CDialog)
MediaSwarmerDlg::MediaSwarmerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MediaSwarmerDlg::IDD, pParent)
{
}

MediaSwarmerDlg::~MediaSwarmerDlg()
{
}

void MediaSwarmerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MEDIAS_LIST, m_list_control);
}

BOOL MediaSwarmerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_control.InsertColumn(0,"Messages",LVCFMT_LEFT,1000);
	return true;
}

BEGIN_MESSAGE_MAP(MediaSwarmerDlg, CDialog)
END_MESSAGE_MAP()


// MediaSwarmerDlg message handlers
