// SupplyProcessorTestDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "SupplyProcessor.h"
#include "SupplyProcessorTestDlg.h"
#include "SupplyProcessorDLL.h"


// SupplyProcessorTestDlg dialog

IMPLEMENT_DYNAMIC(SupplyProcessorTestDlg, CDialog)
SupplyProcessorTestDlg::SupplyProcessorTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SupplyProcessorTestDlg::IDD, pParent)
{
}

SupplyProcessorTestDlg::~SupplyProcessorTestDlg()
{
}

void SupplyProcessorTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST4, m_list_ctrl);
	DDX_Control(pDX, IDC_LIST3, m_queue_list_ctrl);
}

void SupplyProcessorTestDlg::InitParent(SupplyProcessorDLL *parent)
{
	p_dll = parent;
}


BEGIN_MESSAGE_MAP(SupplyProcessorTestDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// SupplyProcessorTestDlg message handlers
BOOL SupplyProcessorTestDlg::OnInitDialog()
{

	CDialog::OnInitDialog();
	//Init any crap here
	m_list_ctrl.InsertColumn(0, "Status Messages", LVCFMT_LEFT,450);
	m_queue_list_ctrl.InsertColumn(0, "Queue", LVCFMT_LEFT,450);
	return (true);
}


void SupplyProcessorTestDlg::Log(const char *buf)
{
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

void SupplyProcessorTestDlg::Queue(const char *buf)
{
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

	int count = m_queue_list_ctrl.GetItemCount();
	if(count > 1000) m_queue_list_ctrl.DeleteItem(1000);

	m_queue_list_ctrl.InsertItem(0, msg);
}

void SupplyProcessorTestDlg::ClearQueue()
{
	m_queue_list_ctrl.DeleteAllItems();
}

UINT SendThreadProc(LPVOID p_dll)
{
	((SupplyProcessorDLL*)p_dll)->Send();
	return 1;
}

UINT ClearDBThreadProc(LPVOID p_dll)
{
	((SupplyProcessorDLL*)p_dll)->ClearDB();
	return 1;
}


void SupplyProcessorTestDlg::OnTimer(UINT nIDEvent)
{
	TRACE("ON TIMER %u\n", nIDEvent);
	if(nIDEvent == 1) {
		Log("Starting ClearDB Thread");
		SetTimer(1, 24*60*60*1000, NULL);
		AfxBeginThread(ClearDBThreadProc,(LPVOID)p_dll,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
	}

	if(nIDEvent == 2) {
		Log("Starting Send Thread");
		AfxBeginThread(SendThreadProc,(LPVOID)p_dll,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
	}
	
	CDialog::OnTimer(nIDEvent);

}


