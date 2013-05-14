// KazaaLauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "KazaaLauncherDlg.h"
#include "KazaaLauncherDll.h"


// CKazaaLauncherDlg dialog

IMPLEMENT_DYNAMIC(CKazaaLauncherDlg, CDialog)
CKazaaLauncherDlg::CKazaaLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKazaaLauncherDlg::IDD, pParent)
{
}

CKazaaLauncherDlg::~CKazaaLauncherDlg()
{
}

void CKazaaLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SUPERNODE_LIST, m_supernode_list);
	DDX_Control(pDX, IDC_Log_LIST, m_log_list);
}


BEGIN_MESSAGE_MAP(CKazaaLauncherDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_UPDATE_BUTTON, OnBnClickedUpdateButton)
	ON_WM_TIMER()
	ON_MESSAGE(WM_THREAD_DONE,GotThreadDoneMessage)
	ON_MESSAGE(WM_KILL_KAZAA, OnKillKazaaMessage)
	ON_MESSAGE(WM_LOG, OnLogMessage)
	

	ON_BN_CLICKED(IDC_Kill_All_Kazaa_BUTTON, OnBnClickedKillAllKazaaButton)
	ON_BN_CLICKED(IDC_Minimize_All_Kazaa_BUTTON, OnBnClickedMinimizeAllKazaaButton)
	ON_BN_CLICKED(IDC_RESTORE_BUTTON, OnBnClickedRestoreButton)
END_MESSAGE_MAP()


// CKazaaLauncherDlg message handlers

BOOL CKazaaLauncherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CRect main_rect,list_rect,button_rect;
	GetWindowRect(&main_rect);
	GetDlgItem(IDC_SUPERNODE_LIST)->GetWindowRect(&list_rect);
	m_border=list_rect.left-main_rect.left;
	m_bottom=main_rect.bottom - list_rect.bottom;

	m_supernode_list.SetExtendedStyle(m_supernode_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_supernode_list.InsertColumn(SUB_PROCESS_ID,"Kazaa PID",LVCFMT_LEFT,80);
	m_supernode_list.InsertColumn(SUB_CONNECTING_SUPERNODE_IP,"Connecting Supernode",LVCFMT_LEFT,140);
	m_supernode_list.InsertColumn(SUB_CONNECTED_SUPERNODE_IP,"Connected Supernode",LVCFMT_LEFT,140);
	m_supernode_list.InsertColumn(3,"Running In",LVCFMT_LEFT,180);
	m_supernode_list.InsertColumn(4,"Desktop",LVCFMT_LEFT,60);
	m_supernode_list.InsertColumn(5,"Collector",LVCFMT_LEFT,100);


	m_log_list.SetHorizontalExtent(1000);

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CKazaaLauncherDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(IsWindowVisible())
	{
		CRect main_rect,list_rect,button_rect;

		GetWindowRect(&main_rect);
		GetDlgItem(IDC_SUPERNODE_LIST)->GetWindowRect(&list_rect);
		list_rect.bottom=main_rect.bottom-m_border-m_bottom;
		list_rect.right=main_rect.right-m_border;
		ScreenToClient(&list_rect);
		GetDlgItem(IDC_SUPERNODE_LIST)->MoveWindow(&list_rect);

		GetDlgItem(IDC_Log_LIST)->GetWindowRect(&list_rect);
		list_rect.right=main_rect.right-m_border;
		ScreenToClient(&list_rect);
		GetDlgItem(IDC_Log_LIST)->MoveWindow(&list_rect);
	}
}

//
//
//
void CKazaaLauncherDlg::InitParent(KazaaLauncherDll* parent)
{
	p_parent = parent;
}

//
//
//
void CKazaaLauncherDlg::OnBnClickedUpdateButton()
{
	m_supernode_list.DeleteAllItems();
	p_parent->EnumAllKazaaProcesses();
	p_parent->GetAllKazaaCurrentSupernodes();
}

//
//
//
void CKazaaLauncherDlg::RemoveKazaa(DWORD process_id)
{
	int count = m_supernode_list.GetItemCount();							   
	
	HWND hList = m_supernode_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = process_id;
	int find = ListView_FindItem(hList, -1, &lvfi);
	if(find >= 0)
	{
		m_supernode_list.DeleteItem(find);
	}
	char msg[32];
	sprintf(msg, "Processes: %d", m_supernode_list.GetItemCount());
	GetDlgItem(IDC_PROCESSES_STATIC)->SetWindowText(msg);
}

//
//
//
void CKazaaLauncherDlg::UpdateSupernodeList(KazaaProcess& process) //status 0 = connecting, status 1 = connected, status -1 = Not connected
{																			   
	int count = m_supernode_list.GetItemCount();							   
	
	HWND hList = m_supernode_list.GetSafeHwnd();
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = process.m_process_id;
	int find = ListView_FindItem(hList, -1, &lvfi);
	if(find >= 0)
	{
		char ip[32];
		char num[4];
		sprintf(ip, "%s:%d", process.m_connecting_supernode_ip.c_str(), process.m_connecting_supernode_port);
		m_supernode_list.SetItemText(find, SUB_CONNECTING_SUPERNODE_IP, ip);
		sprintf(ip, "%s:%d", process.m_connected_supernode_ip.c_str(), process.m_connected_supernode_port);
		m_supernode_list.SetItemText(find, SUB_CONNECTED_SUPERNODE_IP, ip);

		m_supernode_list.SetItemText(find,3,process.m_running_dir);
		sprintf(num,"%d",process.m_desktop_num);
		m_supernode_list.SetItemText(find,4,num);
		if(process.m_has_reported_to_kazaa_supply_taker)
			m_supernode_list.SetItemText(find,5,"1");
		else
			m_supernode_list.SetItemText(find,5,"0");
	}
	else
	{
		char process_id[8];
		char ip[32];
		char num[4];
		sprintf(process_id, "%d", process.m_process_id);
		int index =  m_supernode_list.InsertItem(count, process_id);
		m_supernode_list.SetItemData(index, process.m_process_id);

		sprintf(ip, "%s:%d", process.m_connecting_supernode_ip.c_str(), process.m_connecting_supernode_port);
		m_supernode_list.SetItemText(index, SUB_CONNECTING_SUPERNODE_IP, ip);
		sprintf(ip, "%s:%d", process.m_connected_supernode_ip.c_str(), process.m_connected_supernode_port);
		m_supernode_list.SetItemText(index, SUB_CONNECTED_SUPERNODE_IP, ip);

		m_supernode_list.SetItemText(index,3,process.m_running_dir);
		sprintf(num,"%d",process.m_desktop_num);
		m_supernode_list.SetItemText(index,4,num);
		if(process.m_has_reported_to_kazaa_supply_taker)
			m_supernode_list.SetItemText(index,5,"1");
		else
			m_supernode_list.SetItemText(index,5,"0");
	}

	char msg[32];
	sprintf(msg, "Processes: %d", m_supernode_list.GetItemCount());
	GetDlgItem(IDC_PROCESSES_STATIC)->SetWindowText(msg);
}
//
//
//
void CKazaaLauncherDlg::OnTimer(UINT nIDEvent)
{
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
LRESULT CKazaaLauncherDlg::GotThreadDoneMessage(WPARAM wparam,LPARAM lparam)
{
	p_parent->GotThreadDoneMessage();
	return 0;
}

//
//
//
LRESULT CKazaaLauncherDlg::OnKillKazaaMessage(WPARAM wparam,LPARAM lparam)
{
	DWORD pid = (DWORD)wparam;
	p_parent->KillKazaa(pid);
	return 0;
}

//
//
//
LRESULT CKazaaLauncherDlg::OnLogMessage(WPARAM wparam,LPARAM lparam)
{
	const char* log = (const char*)wparam;
	Log(log);
	return 0;
}

//
//
//
void CKazaaLauncherDlg::Log(const char * text)
{
	CString msg = (CTime::GetCurrentTime()).Format("%Y-%m-%d %H:%M:%S - ");
	msg += text;
	if(m_log_list.GetCount()>50000)
	{
		m_log_list.DeleteString(m_log_list.GetCount()-1);
	}
	m_log_list.InsertString(0,msg);
}

//
//
//
void CKazaaLauncherDlg::OnBnClickedKillAllKazaaButton()
{
	m_supernode_list.DeleteAllItems();
	p_parent->KillAllKazaaProcesses();
	p_parent->ResumeLaunchingKazaa();
}

//
//
//
void CKazaaLauncherDlg::OnBnClickedMinimizeAllKazaaButton()
{
	p_parent->MinimizeAllKazaaWindows();
}

//
//
//
void CKazaaLauncherDlg::OnBnClickedRestoreButton()
{
	p_parent->RestoreAllKazaaWindows();
}
