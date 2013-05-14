// MainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MainDlg.h"
#include "SupernodeControllerDll.h"

// CMainDlg dialog

IMPLEMENT_DYNAMIC(CMainDlg, CDialog)
CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	m_remote_supernode_list_count=m_check_new_supernode_count=m_request_new_supernode_count=m_remove_supernode_count=0;
	m_supernode_taken=m_duplicated_supernode=m_supernode_removed=0;
}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_log_list);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SEND_INIT_BUTTON, OnBnClickedSendInitButton)
	ON_BN_CLICKED(IDC_RACK_STATUS_BUTTON, OnBnClickedRackStatusButton)
	ON_BN_CLICKED(IDC_IS_SORTED_BUTTON, OnBnClickedIsSortedButton)
	ON_BN_CLICKED(IDC_RESET_BUTTON, OnBnClickedResetButton)
	ON_BN_CLICKED(IDC_SAVE_SUPERNODES_BUTTON, OnBnClickedSaveSupernodesButton)
	ON_BN_CLICKED(IDC_RESET_SHARED_FOLDERS_BUTTON, OnBnClickedResetSharedFoldersButton)
END_MESSAGE_MAP()


// CMainDlg message handlers
//
//
void CMainDlg::InitParent(SupernodeControllerDll* parent)
{
	p_parent = parent;
	m_rack_status_dlg.InitParent(parent);
}

//
//
//
void CMainDlg::OnTimer(UINT nIDEvent)
{
	if(nIDEvent == 3) //  one second time
	{
		RefreshControllerMessagesStatus();
	}
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CMainDlg::Log(const char * text)
{
	if(m_log_list.GetCount()>50000)
	{
		m_log_list.DeleteString(m_log_list.GetCount()-1);
	}
	m_log_list.InsertString(0,text);
}

//
//
//
void CMainDlg::RefreshControllerMessagesStatus()
{
	char msg[128];
	sprintf(msg, "Remote Supernode List: %u", m_remote_supernode_list_count);
	GetDlgItem(IDC_REMOTE_SUPERNODE_LIST_STATIC)->SetWindowText(msg);

	sprintf(msg, "Check New Supernode: %u", m_check_new_supernode_count);
	GetDlgItem(IDC_CHECK_NEW_SUPERNODE_STATIC)->SetWindowText(msg);

	sprintf(msg, "Request New Supernode: %u", m_request_new_supernode_count);
	GetDlgItem(IDC_REQUEST_NEW_SUPERNODE_STATIC)->SetWindowText(msg);

	sprintf(msg, "Remove Supernode: %u", m_remove_supernode_count);
	GetDlgItem(IDC_REMOVE_SUPERNODE_STATIC)->SetWindowText(msg);

	sprintf(msg, "Supernode Taken: %u", m_supernode_taken);
	GetDlgItem(IDC_SUPERNODE_TAKEN_STATIC)->SetWindowText(msg);

	sprintf(msg, "Duplicated Supernode: %u", m_duplicated_supernode);
	GetDlgItem(IDC_DUPLICATED_SUPERNODE_STATIC)->SetWindowText(msg);

	sprintf(msg, "Max Duplicated Supernode Assigned: %u", p_parent->m_max_same_supernode_assigned);
	GetDlgItem(IDC_MAX_DUPLICATED_ASSIGNED_STATIC)->SetWindowText(msg);

	sprintf(msg, "Supernodes Removed: %u", m_supernode_removed);
	GetDlgItem(IDC_SUPERNODE_REMOVED_STATIC)->SetWindowText(msg);

}
//
//
//
void CMainDlg::UpdateStatus(int total_collectors, int up_collectors, int supernodes_collected, int supernodes_consumed, int num_launchers,
							int launcher_supernodes)
{
	char msg[128];
	sprintf(msg, "Total Collectors: %d", total_collectors);
	GetDlgItem(IDC_TOTAL_COLLECTORS_STATIC)->SetWindowText(msg);
	
	sprintf(msg, "Collectors are Up: %d", up_collectors);
	GetDlgItem(IDC_COLLECTORS_UP_STATIC)->SetWindowText(msg);
	
	sprintf(msg, "Supernodes Collected (main): %d", supernodes_collected);
	GetDlgItem(IDC_SUPERNODES_COLLECTED_STATIC)->SetWindowText(msg);
	
	sprintf(msg, "Supernodes Assigned (main) : %d", supernodes_consumed);
	GetDlgItem(IDC_SUPERNODES_USED_STATIC)->SetWindowText(msg);
	
	sprintf(msg, "Number of Launcher: %d", num_launchers);
	GetDlgItem(IDC_NUM_LAUNCHER_STATIC)->SetWindowText(msg);

	sprintf(msg, "Total Supernodes: %d", launcher_supernodes);
	GetDlgItem(IDC_TOTAL_SUPERNODES_STATIC)->SetWindowText(msg);

}

//
//
//
void CMainDlg::OnBnClickedSendInitButton()
{
	p_parent->SendInitMsgToAll();
}

//
//
//
void CMainDlg::OnBnClickedRackStatusButton()
{
	m_rack_status_dlg.ShowWindow(SW_NORMAL);
	m_rack_status_dlg.BringWindowToTop();
}

//
//
//
BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_rack_status_dlg.Create(IDD_RACK_STATUS_DIALOG,CWnd::GetDesktopWindow());
	m_rack_status_dlg.ShowWindow(SW_HIDE);
	m_log_list.SetHorizontalExtent(1000);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CMainDlg::OnBnClickedIsSortedButton()
{
	UINT max = p_parent->GetMaxHashSetSize();
	char msg[256];
	sprintf(msg, "Maximum Hash Set size is %u.",max);
	MessageBox(msg,"OK",MB_OK);	
}

//
//
//
void CMainDlg::OnBnClickedResetButton()
{
	p_parent->ResetAll();
}

//
//
//
void CMainDlg::OnBnClickedSaveSupernodesButton()
{
	p_parent->SaveSupernodes();
}

//
//
//
void CMainDlg::OnBnClickedResetSharedFoldersButton()
{
	p_parent->ReadAndSendAdditionalSharedFolders();
}
