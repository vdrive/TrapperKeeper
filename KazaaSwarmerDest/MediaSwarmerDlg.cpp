// MediaSwarmerDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "KazaaSwarmerDest.h"
#include "MediaSwarmerDlg.h"
#include "MediaSwarmer.h"
#include "..\tkcom\Timer.h"


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
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void MediaSwarmerDlg::InitParent(MediaSwarmer* parent)
{
	p_parent = parent;
}



void MediaSwarmerDlg::OnTimer(UINT nIDEvent)
{
	static Timer m_rescan_timer;
	static bool b_has_scanned_once=false;
	switch (nIDEvent)
	{
		case 1:
		{
			if(!b_has_scanned_once || m_rescan_timer.HasTimedOut(60*60*8)){  //production
			//if(!b_has_scanned_once || m_rescan_timer.HasTimedOut(30)){  //debug
				m_rescan_timer.Refresh();
				b_has_scanned_once=true;
				p_parent->StartThread();

				//p_parent->ReadDestDir();
			}
			break;
		}
		default :
		{
			break;
		}
	}
}

// MediaSwarmerDlg message handlers

void MediaSwarmerDlg::OnDestroy()
{
	CDialog::OnDestroy();
}
