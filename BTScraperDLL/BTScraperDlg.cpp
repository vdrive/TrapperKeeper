// BTScraperDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTScraperModDLL.h"
#include "BTScraperDlg.h"


// BTScraperDlg dialog

IMPLEMENT_DYNAMIC(BTScraperDlg, CDialog)
BTScraperDlg::BTScraperDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BTScraperDlg::IDD, pParent)
{
}

BTScraperDlg::~BTScraperDlg()
{
}

bool BTScraperDlg::SendReliableData(char * dest, void* send_data, UINT data_length) 
{
	return p_dll->p_com_interface->SendReliableData(dest, (void*)send_data, data_length);
}

void BTScraperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST4, m_main_list_ctrl);
	DDX_Control(pDX, IDC_LIST5, m_resend_list_ctrl);
	DDX_Control(pDX, IDC_LIST6, m_recheck_list_ctrl);
}

void BTScraperDlg::InitParent(BTScraperMod *parent)
{
	p_dll = parent;
}



BEGIN_MESSAGE_MAP(BTScraperDlg, CDialog)
	ON_WM_TIMER()
	ON_LBN_SELCHANGE(IDC_LIST4, OnLbnSelchangeList1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST4, OnLvnItemchangedList4)
END_MESSAGE_MAP()


// BTScraperDlg message handlers

BOOL BTScraperDlg::OnInitDialog()
{

	CDialog::OnInitDialog();
	//Init any crap here
	m_main_list_ctrl.InsertColumn(0, "Messages", LVCFMT_LEFT,450);
	m_resend_list_ctrl.InsertColumn(0, "Resend Messages", LVCFMT_LEFT,450);
	m_recheck_list_ctrl.InsertColumn(0, "Recheck Messages", LVCFMT_LEFT,450);
	return (true);
}



void BTScraperDlg::Log(const char * buf, int type)
{
	CListCtrl * list_ctrl = NULL;
	switch(type) {	
		case(MAIN_MESSAGE):
			list_ctrl = &m_main_list_ctrl;
			break;
		case(RESEND_MESSAGE):
			list_ctrl = &m_resend_list_ctrl;
			break;
		case(RECHECK_MESSAGE):
			list_ctrl = &m_recheck_list_ctrl;
			break;
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

	int count = list_ctrl->GetItemCount();
	if(count > 1000) list_ctrl->DeleteItem(1000);

	list_ctrl->InsertItem(0, msg);
}

UINT CollectThreadProc(LPVOID p_dll)
{
	((BTScraperMod*)p_dll)->Collect();
	return 1;
}



void BTScraperDlg::OnTimer(UINT nIDEvent)
{
	//p_dll->OnTimer(nIDEvent);

	if(nIDEvent == 1) {
		AfxBeginThread(CollectThreadProc,(LPVOID)p_dll,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
		
	}
	
	CDialog::OnTimer(nIDEvent);

}


void BTScraperDlg::OnLbnSelchangeList1()
{
	// TODO: Add your control notification handler code here
}

void BTScraperDlg::OnLvnItemchangedList4(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
