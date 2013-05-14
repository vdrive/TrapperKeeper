// BTRemoverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTRemoverDll.h"
#include "BTRemoverDlg.h"
#include "WebRemover.h"


// BTRemoverDlg dialog

IMPLEMENT_DYNAMIC(BTRemoverDlg, CDialog)
BTRemoverDlg::BTRemoverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BTRemoverDlg::IDD, pParent)
{
	p_dll = NULL;
}

BTRemoverDlg::~BTRemoverDlg()
{
}

void BTRemoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_main_list_ctrl);
}


BEGIN_MESSAGE_MAP(BTRemoverDlg, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, OnLvnItemchangedList2)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
END_MESSAGE_MAP()


// BTRemoverDlg message handlers

void BTRemoverDlg::OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

BOOL BTRemoverDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//Init any crap here
	m_main_list_ctrl.InsertColumn(0, "Messages", LVCFMT_LEFT,450);
	return (true);
}


void BTRemoverDlg::Log(const char * buf)
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

	int count = m_main_list_ctrl.GetItemCount();
	if(count > 1000) m_main_list_ctrl.DeleteItem(1000);

	m_main_list_ctrl.InsertItem(0, msg);
}

UINT CollectThreadProc(LPVOID p_dll)
{
	((BTRemoverDll*)p_dll)->Collect();
	return 1;
}

void BTRemoverDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	Log("Search clicked");
	AfxBeginThread(CollectThreadProc,(LPVOID)p_dll,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
}

void BTRemoverDlg::SetDll(BTRemoverDll * dll)
{
	p_dll = dll;
}

void BTRemoverDlg::OnBnClickedOk2()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
