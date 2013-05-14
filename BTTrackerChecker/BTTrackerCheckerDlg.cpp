// BTTrackerCheckerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTTrackerCheckerDll.h"
#include "BTTrackerCheckerDlg.h"


// BTTrackerCheckerDlg dialog

IMPLEMENT_DYNAMIC(BTTrackerCheckerDlg, CDialog)
BTTrackerCheckerDlg::BTTrackerCheckerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BTTrackerCheckerDlg::IDD, pParent)
{
	p_dll = NULL;
	b_init = false;
}

BTTrackerCheckerDlg::~BTTrackerCheckerDlg()
{
	if(p_dll != NULL) {
		//delete p_dll;
		//p_dll = NULL;
	}
}

void BTTrackerCheckerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_list_ctrl);
}


BEGIN_MESSAGE_MAP(BTTrackerCheckerDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL BTTrackerCheckerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_ctrl.InsertColumn(0, "Messages", LVCFMT_LEFT,450);
	b_init = true;
	return (true);
}

// BTTrackerCheckerDlg message handlers


UINT CheckThreadProc(LPVOID p_dll)
{
	((BTTrackerCheckerDll*)p_dll)->CheckTracker();
	return 1;
}

UINT RecheckThreadProc(LPVOID p_dll)
{
	((BTTrackerCheckerDll*)p_dll)->RecheckTracker();
	return 1;
}

void BTTrackerCheckerDlg::OnTimer(UINT nIDEvent)
{
	//p_dll->OnTimer(nIDEvent);

	if(nIDEvent == 2) {
		Log("Starting Check Thread");
		AfxBeginThread(CheckThreadProc,(LPVOID)p_dll,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
	}
	if(nIDEvent == 3) {
		AfxBeginThread(RecheckThreadProc,(LPVOID)p_dll,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
	}
	
	CDialog::OnTimer(nIDEvent);

}


void BTTrackerCheckerDlg::Log(string buf)
{
	Log(buf.c_str());
}
void BTTrackerCheckerDlg::Log(const char * buf)
{

	if(!b_init) {
		TRACE("LOG: %s\n", buf);
		return;
	}
	char msg[4096];
	if(strlen(buf) > 4005) {
		memcpy(msg, buf, 4005);
		msg[4005] = '\0';
		Log(msg);
		return;
	}
	CTime now=CTime::GetCurrentTime();
	sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
	strcat(msg, buf);

	int count = m_list_ctrl.GetItemCount();
	if(count > 1000) m_list_ctrl.DeleteItem(1000);

	m_list_ctrl.InsertItem(0, msg);
}
