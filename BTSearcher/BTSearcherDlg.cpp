// BTSearcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTSearcherDll.h"
#include "BTSearcherDlg.h"
#include "ReportGenerator.h"
#include "WebSearcher.h"


// BTSearcherDlg dialog

IMPLEMENT_DYNAMIC(BTSearcherDlg, CDialog)
BTSearcherDlg::BTSearcherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BTSearcherDlg::IDD, pParent)
{
	p_dll = NULL;
}

BTSearcherDlg::~BTSearcherDlg()
{
}

void BTSearcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_main_list_ctrl);
}


BEGIN_MESSAGE_MAP(BTSearcherDlg, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, OnLvnItemchangedList2)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
END_MESSAGE_MAP()


// BTSearcherDlg message handlers

void BTSearcherDlg::OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

BOOL BTSearcherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//Init any crap here
	m_main_list_ctrl.InsertColumn(0, "Messages", LVCFMT_LEFT,450);
	return (true);
}


void BTSearcherDlg::Log(const char * buf)
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
	((BTSearcherDll*)p_dll)->Collect();
	return 1;
}

UINT GenerateThreadProc(LPVOID p_dll)
{
	ReportGenerator gen(&((BTSearcherDlg*)p_dll)->m_main_list_ctrl);
	vector<string> v_labels;
	//v_labels.push_back("Fox");
	v_labels.push_back("Universal Music");
	//v_labels.push_back("Universal Pictures");
	gen.GenerateReport(v_labels);
	TRACE("DONE\n");
	((BTSearcherDlg*)p_dll)->Log("Done");
	return 1;

}
void BTSearcherDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	Log("Search clicked");
	AfxBeginThread(CollectThreadProc,(LPVOID)p_dll,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
}

void BTSearcherDlg::SetDll(BTSearcherDll * dll)
{
	p_dll = dll;
}
void BTSearcherDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	TRACE("Generating Report");
	Log("Generating Report");
	AfxBeginThread(GenerateThreadProc,(LPVOID)this,THREAD_PRIORITY_BELOW_NORMAL,0,0,NULL);
}

void BTSearcherDlg::OnBnClickedOk2()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
