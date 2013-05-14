// UsenetPosterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UsenetPosterDll.h"
#include "UsenetPosterDlg.h"


// UsenetPosterDlg dialog

IMPLEMENT_DYNAMIC(UsenetPosterDlg, CDialog)
UsenetPosterDlg::UsenetPosterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(UsenetPosterDlg::IDD, pParent)
{
}

UsenetPosterDlg::~UsenetPosterDlg()
{
}

void UsenetPosterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list_box);
	DDX_Control(pDX, IDC_LIST_SERVERS, m_servers_list);
}

//
//
//
void UsenetPosterDlg::InitParent(UsenetPosterDll *dll)
{
	p_dll = dll;
}

//
//
//
void UsenetPosterDlg::Log(char *buf)
{
	char msg[4096];
	CTime now=CTime::GetCurrentTime();
	sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
	strcat(msg, buf);

	m_list_box.InsertString(0,msg);
	while(m_list_box.GetCount()>1000)
	{
		m_list_box.DeleteString(m_list_box.GetCount()-1);
	}
}


BEGIN_MESSAGE_MAP(UsenetPosterDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// UsenetPosterDlg message handlers

void UsenetPosterDlg::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
	case 1:
		{
			// This is the watchdog timer, go through the sockets and test only the ones which are activly posting...
			for (int i=0; i < (int)p_dll->v_usenet_socket_ptrs.size(); i++)
			{
				if (p_dll->v_usenet_socket_ptrs[i]->m_posting && p_dll->v_usenet_socket_ptrs[i]->m_is_active)
				{
					CTimeSpan ts = CTime::GetCurrentTime() - p_dll->v_usenet_socket_ptrs[i]->m_last_server_response_time;
					if (ts.GetTotalMinutes() > 1)
					{
						p_dll->Log(p_dll->v_usenet_socket_ptrs[i]->m_nickname, "WATCHDOG FIRED.  Re-connecting...");
						p_dll->StartPosting(p_dll->v_usenet_socket_ptrs[i]);
					}
				}
			}
			break;
		}
	default:
		{
			break;
		}
	}

	CDialog::OnTimer(nIDEvent);
}
