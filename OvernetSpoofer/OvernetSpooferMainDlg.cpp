// OvernetSpooferMainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OvernetSpooferMainDlg.h"
#include "OvernetSpooferDll.h"


// COvernetSpooferMainDlg dialog

IMPLEMENT_DYNAMIC(COvernetSpooferMainDlg, CDialog)
COvernetSpooferMainDlg::COvernetSpooferMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COvernetSpooferMainDlg::IDD, pParent)
{
}

COvernetSpooferMainDlg::~COvernetSpooferMainDlg()
{
}

void COvernetSpooferMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_log_list);
}


BEGIN_MESSAGE_MAP(COvernetSpooferMainDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RESET_LOG_BUTTON, OnBnClickedResetLogButton)
END_MESSAGE_MAP()


// COvernetSpooferMainDlg message handlers
void COvernetSpooferMainDlg::InitParent(OvernetSpooferDll* parent)
{
	p_parent=parent;
}

//
//
//
void COvernetSpooferMainDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
BOOL COvernetSpooferMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_log_list.SetHorizontalExtent(2048);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
//
//
//
void COvernetSpooferMainDlg::Log(const char *buf)
{

	CString cstring;
	cstring.Format("%s - %s", CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"),buf);
	cstring.Trim();
	m_log_list.InsertString(0,cstring);

	while(m_log_list.GetCount()>50000)
	{
		m_log_list.DeleteString(m_log_list.GetCount()-1);
	}
}

//
//
//
void COvernetSpooferMainDlg::StatusReady(ConnectionModuleStatusData& status)
{
	CString num;
	num.Format("%u",status.m_num_peers);
	GetDlgItem(IDC_NUM_PEERS_STATIC)->SetWindowText(num);
	num.Format("%u",status.m_num_supplies);
	GetDlgItem(IDC_NUM_SUPPLY_STATIC)->SetWindowText(num);
	num.Format("%u",status.m_num_projects);
	GetDlgItem(IDC_NUM_PROJECTS_STATIC)->SetWindowText(num);
	num.Format("%u",status.m_hash_search_index);
	GetDlgItem(IDC_SEARCHING_INDEX_STATIC)->SetWindowText(num);
	num.Format("%u",status.m_publicize_ack_received);
	GetDlgItem(IDC_NUM_PUBLICIZE_ACKS_STATIC)->SetWindowText(num);
	num.Format("%u",status.m_userhash_to_publicize);
	GetDlgItem(IDC_NUM_USERID_HASH_STATIC)->SetWindowText(num);
	num.Format("%u",status.m_queries_received);
	GetDlgItem(IDC_NUM_QUERIES_RECEIVED_STATIC)->SetWindowText(num);
	num.Format("%u",status.m_publish_received);
	GetDlgItem(IDC_NUM_PUBLISH_RECEIVED_STATIC)->SetWindowText(num);
	num.Format("%u",status.m_spoofs_sent);
	GetDlgItem(IDC_SPOOFS_SENT_STATIC)->SetWindowText(num);
}

//
//
//
void COvernetSpooferMainDlg::OnBnClickedResetLogButton()
{
	m_log_list.ResetContent();
}
