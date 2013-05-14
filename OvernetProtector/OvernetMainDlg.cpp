// OvernetMainDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OvernetProtectorDll.h"
#include "OvernetMainDlg.h"

// COvernetMainDlg dialog

IMPLEMENT_DYNAMIC(COvernetMainDlg, CDialog)
COvernetMainDlg::COvernetMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COvernetMainDlg::IDD, pParent)
{
}

COvernetMainDlg::~COvernetMainDlg()
{
}

void COvernetMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_log_list);
	DDX_Control(pDX, IDC_SUPPLY_PROGRESS2, m_supply_entry_progress);
}


BEGIN_MESSAGE_MAP(COvernetMainDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// COvernetMainDlg message handlers
void COvernetMainDlg::InitParent(OvernetProtectorDll* parent)
{
	p_parent=parent;
}

void COvernetMainDlg::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
void COvernetMainDlg::Log(const char *buf)
{

	CString cstring;
	cstring.Format("%s - %s", CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"),buf);
	cstring.Trim();
	m_log_list.InsertString(m_log_list.GetCount(),cstring);

	while(m_log_list.GetCount()>50000)
	{
		m_log_list.DeleteString(0);
	}
}

//
//
//
void COvernetMainDlg::StatusReady(ConnectionModuleStatusData& status)
{
	m_num_results+=status.v_supply_data.size();
	CString num;
	num.Format("%u",status.m_num_peers);
	GetDlgItem(IDC_NUM_PEERS_STATIC)->SetWindowText(num);
	num.Format("%u",m_num_results);
	GetDlgItem(IDC_STATIC_NUM_RESULTS)->SetWindowText(num);	
}

//
//
//
BOOL COvernetMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_num_results=0;
	m_log_list.SetHorizontalExtent(2048);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void COvernetMainDlg::SpoofEntriesRetrievalStarted()
{
	CString done = (CTime::GetCurrentTime()).Format("Last Supply Process Started at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_RETRIEVAL_PROCESS_STARTED_STATIC)->SetWindowText(done);
	GetDlgItem(IDC_SUPPLY_ENTRY_PERCENT_STATIC)->SetWindowText("0 %");
	m_supply_entry_progress.SetPos(0);
}

//
//
//
void COvernetMainDlg::RetrievingProject(const char* status,int percent)
{
	GetDlgItem(IDC_SUPPLY_ENTRY_STATIC)->SetWindowText(status);
	m_supply_entry_progress.SetPos(percent);
	CString msg;
	msg.Format("%d %%", percent);
	GetDlgItem(IDC_SUPPLY_ENTRY_PERCENT_STATIC)->SetWindowText(msg);

}

//
//
//
void COvernetMainDlg::DoneRetrievingProject()
{
	CString done = (CTime::GetCurrentTime()).Format("Last Supply Process Ended at %Y-%m-%d %H:%M:%S");
	GetDlgItem(IDC_RETRIEVAL_PROCESS_ENDED_STATIC)->SetWindowText(done);
	GetDlgItem(IDC_SUPPLY_ENTRY_STATIC)->SetWindowText("");
	m_supply_entry_progress.SetPos(100);
	GetDlgItem(IDC_SUPPLY_ENTRY_PERCENT_STATIC)->SetWindowText("100 %");
}
