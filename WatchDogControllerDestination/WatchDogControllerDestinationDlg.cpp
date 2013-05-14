// WatchDogControllerDestinationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WatchDogControllerDestinationDlg.h"
#include "WatchDogControllerDestinationDll.h"


// CWatchDogControllerDestinationDlg dialog

IMPLEMENT_DYNAMIC(CWatchDogControllerDestinationDlg, CDialog)
CWatchDogControllerDestinationDlg::CWatchDogControllerDestinationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWatchDogControllerDestinationDlg::IDD, pParent)
{
}

CWatchDogControllerDestinationDlg::~CWatchDogControllerDestinationDlg()
{
}

void CWatchDogControllerDestinationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);	
	DDX_Control(pDX, IDC_LIST, m_list_box);
}


BEGIN_MESSAGE_MAP(CWatchDogControllerDestinationDlg, CDialog)
	ON_WM_TIMER()
	ON_LBN_SELCHANGE(IDC_LIST, OnLbnSelchangeList)
END_MESSAGE_MAP()

//
//
//
void CWatchDogControllerDestinationDlg::InitParent(WatchDogControllerDestinationDll * parent)
{
	p_parent = parent;
}

//
//
//
void CWatchDogControllerDestinationDlg::OnTimer(UINT nIDEvent)
{
	CDialog::OnTimer(nIDEvent);

	if (nIDEvent == 1)
	{
		p_parent->RestartComputer();
	}
}

//
//
// return message when it recieved a ping message from source
void CWatchDogControllerDestinationDlg::ReturnMessage(char *ip)
{
	CTime m_time = CTime::GetCurrentTime();
	//CString str_time = cur_time.Format(" on %d %b %H:%M:%S");
	char buf[1024];
	CString str = "Recieved a ping message from ";

	int hour = m_time.GetHour();
	if (hour < 12) // 0-11 A.M
	{
		if (hour == 0) // midnight
			hour = 12;

		sprintf(buf, " on %u/%02u - %u:%02u:%02u AM", m_time.GetMonth(), m_time.GetDay(), hour, m_time.GetMinute(), m_time.GetSecond());
	}
	else // if PM
	{
		if (hour != 12)
			hour -= 12;
		sprintf(buf, " on %u/%02u - %u:%02u:%02u PM", m_time.GetMonth(), m_time.GetDay(), hour, m_time.GetMinute(), m_time.GetSecond());
	}

	int selected_count = m_list_box.GetCount();
	if (selected_count == 1000)
	{	
		m_list_box.DeleteString(999);
	}
	else
		m_list_box.InsertString(0, str + ip + buf);
}

//
//
//
BOOL CWatchDogControllerDestinationDlg::OnInitDialog(void)
{
	CDialog::OnInitDialog();
	return FALSE;
}

void CWatchDogControllerDestinationDlg::OnLbnSelchangeList()
{
	// TODO: Add your control notification handler code here
}
