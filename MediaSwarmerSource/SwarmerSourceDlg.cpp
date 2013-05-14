// SwarmerSourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MediaSwarmerSource.h"
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
	DDX_Control(pDX, IDC_MEDIASWARMER_LIST, m_list_control);
}


BEGIN_MESSAGE_MAP(SwarmerSourceDlg, CDialog)
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
			p_parent->CopyCompletedDownloads();
			break;
		}
		default :
		{
			break;
		}
	}
}