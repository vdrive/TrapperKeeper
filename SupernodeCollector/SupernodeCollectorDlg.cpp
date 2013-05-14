// SupernodeCollectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SupernodeCollectorDll.h"
#include "SupernodeCollectorDlg.h"

// CSupernodeCollectorDlg dialog

IMPLEMENT_DYNAMIC(CSupernodeCollectorDlg, CDialog)
CSupernodeCollectorDlg::CSupernodeCollectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSupernodeCollectorDlg::IDD, pParent)
{
}

CSupernodeCollectorDlg::~CSupernodeCollectorDlg()
{
}

void CSupernodeCollectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_log_list);
}


BEGIN_MESSAGE_MAP(CSupernodeCollectorDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RESET_BUTTON, OnBnClickedResetButton)
END_MESSAGE_MAP()


// CSupernodeCollectorDlg message handlers
//
//
void CSupernodeCollectorDlg::InitParent(SupernodeCollectorDll* parent)
{
	p_parent = parent;
}

//
//
//
void CSupernodeCollectorDlg::OnTimer(UINT nIDEvent)
{
	p_parent->OnTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}

//
//
//
void CSupernodeCollectorDlg::UpdateConnectionStatus(int connecting, int idle, int timeout, int queue_connecting)
{
	char num[128];
	sprintf(num,"Connecting: %d",connecting);
	SetDlgItemText(IDC_CONNECTING_STATIC, num);
	sprintf(num,"Idle: %d",idle);
	SetDlgItemText(IDC_IDLE_STATIC, num);
	sprintf(num,"Timeout: %d",timeout);
	SetDlgItemText(IDC_TIMEOUT_STATIC, num);
	sprintf(num,"Connecting: %d",queue_connecting);
	SetDlgItemText(IDC_QUEUE_CONNECTING_STATIC, num);


}

//
//
//
void CSupernodeCollectorDlg::UpdateSupernodesStatus(UINT supernodes, UINT queue_size)
{
	char num[128];
	sprintf(num,"Supernodes Collected: %d",supernodes);
	SetDlgItemText(IDC_SUPERNODE_STATIC, num);
	sprintf(num,"Queue Size: %d",queue_size);
	SetDlgItemText(IDC_QUEUE_STATIC, num);
}

//
//
//
void CSupernodeCollectorDlg::Log(CString msg)
{
	if(m_log_list.GetCount()>1000)
	{
		m_log_list.DeleteString(0);
	}
	m_log_list.AddString(msg);
}

//
//
//
void CSupernodeCollectorDlg::OnBnClickedResetButton()
{
	p_parent->ResetSupernodes();
}
