// SwarmerSourceDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "KazaaSwarmerSource.h"
#include "SwarmerSourceDlg.h"
#include "SwarmerSource.h"


// SwarmerSourceDlg dialog

IMPLEMENT_DYNAMIC(SwarmerSourceDlg, CDialog)
SwarmerSourceDlg::SwarmerSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SwarmerSourceDlg::IDD, pParent)
{
}

SwarmerSourceDlg::~SwarmerSourceDlg()
{
}

void SwarmerSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KAZAASWARMER_LIST, m_list_control);
}


BEGIN_MESSAGE_MAP(SwarmerSourceDlg, CDialog)
	ON_BN_CLICKED(IDC_GET_LIST, OnBnClickedGetList)
	ON_BN_CLICKED(IDC_SYNCH_BUTTON,OnBnClickedSynch)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// SwarmerSourceDlg message handlers


void SwarmerSourceDlg::InitParent(SwarmerSource* parent)
{
	p_parent = parent;
}

BOOL SwarmerSourceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_control.InsertColumn(0,"Messages",LVCFMT_LEFT,1000);
	return true;
}


void SwarmerSourceDlg::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
		case 1:
		{
			DisplayInfo("TimerFired");
			p_parent->CopyCompletedDownloads();
			break;
		}

		case 2:
		{
			p_parent->MakeOnTimer();
		}
		default :
		{
			break;
		}
	}
}

void SwarmerSourceDlg::DisplayInfo(char * info)
{
	int index = m_list_control.GetItemCount();
	m_list_control.InsertItem(index,info);
}

void SwarmerSourceDlg::OnBnClickedSynch()
{
	p_parent->CopyCompletedDownloads();
}


void SwarmerSourceDlg::OnBnClickedGetList()
{
	p_parent->GetFileList();
}