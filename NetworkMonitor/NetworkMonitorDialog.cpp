// NetworkMonitorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NetworkMonitorDialog.h"


// CNetworkMonitorDialog dialog

IMPLEMENT_DYNAMIC(CNetworkMonitorDialog, CDialog)
CNetworkMonitorDialog::CNetworkMonitorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CNetworkMonitorDialog::IDD, pParent)
{
	mp_network_system=NULL;
}

CNetworkMonitorDialog::~CNetworkMonitorDialog()
{
}

void CNetworkMonitorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MAINWINDOWFRAME, m_main_window_frame);
	DDX_Control(pDX, IDC_PROCESSTREE, m_process_tree);
	DDX_Control(pDX, IDC_DLLTREE, m_dll_tree);
	DDX_Control(pDX, IDC_SELECTEDRACKDLLs, m_selected_rack_dlls);
	DDX_Control(pDX, IDC_ALIVE, m_alive);
	DDX_Control(pDX, IDC_DEAD, m_dead);
	DDX_Control(pDX, IDC_CRASHED, m_crashed);
	DDX_Control(pDX, IDC_ALIVEPERCENT, m_alive_percent);
	DDX_Control(pDX, IDC_NOTCRASHEDPERCENT, m_not_crashed_percent);
}


BEGIN_MESSAGE_MAP(CNetworkMonitorDialog, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_GLOBALRESTARTDEAD, OnGlobalRestartDead)
	ON_BN_CLICKED(IDC_SELECTEDRESTARTCRASHED, OnSelectedRestartCrashed)
	ON_BN_CLICKED(IDC_SELECTEDRESTARTDEAD, OnSelectedRestartDead)
	ON_BN_CLICKED(IDC_SELECTEDRESTARTALL, OnSelectedRestartAll)
	ON_BN_CLICKED(IDC_SELECTEDREMOTECRASHED, OnSelectedRemoteCrashed)
	ON_BN_CLICKED(IDC_SELECTEDREMOTEDEAD, OnSelectedRemoteDead)
END_MESSAGE_MAP()


// CNetworkMonitorDialog message handlers

BOOL CNetworkMonitorDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect tmp_rect;
	m_main_window_frame.GetWindowRect(&tmp_rect);
	//tmp_rect is in coordinates relative to this windows client rect
	CRect this_global_rect;
	CRect this_client_rect;
	this->GetWindowRect(&this_global_rect);
	this->GetClientRect(&this_client_rect);
	this->ClientToScreen(&this_client_rect);
	tmp_rect.OffsetRect(this_global_rect.left-this_client_rect.left,this_global_rect.top-this_client_rect.top);
	m_main_window_frame.ShowWindow(SW_HIDE);
	
	m_main_window.CreateEx(NULL,NULL,NULL,WS_CLIPCHILDREN|WS_BORDER|WS_VISIBLE|WS_CHILD,tmp_rect,this,0,NULL);

	m_process_tree.SetBkColor(RGB(180,180,180));
	m_dll_tree.SetBkColor(RGB(180,180,180));

	AdjustWindowPositions();
	
	this->SetTimer(1,2000,NULL);
	this->SetTimer(2,60000,NULL);
	this->SetTimer(3,90000,NULL);

	m_main_window.SetDllListBox(m_selected_rack_dlls);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CNetworkMonitorDialog::SetNetworkSystem(NetworkSystem* ns)
{
	mp_network_system=ns;
	m_main_window.SetNetworkSystem(ns);
}

BOOL CNetworkMonitorDialog::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CDialog::OnEraseBkgnd(pDC);
}

void CNetworkMonitorDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	TRACE("CNetworkMonitorDialog::OnSize() %d %d.\n",cx,cy);
	AdjustWindowPositions();
}

void CNetworkMonitorDialog::AdjustWindowPositions(void)
{
	if(!m_main_window.GetSafeHwnd())
		return;
	CRect tmp_rect;
	m_main_window.GetWindowRect(&tmp_rect);

	
	//tmp_rect is in coordinates relative to this windows client rect
	//CRect this_global_rect;
	CRect this_client_rect;
	//this->GetWindowRect(&this_global_rect);
	this->GetClientRect(&this_client_rect);
	//this->ClientToScreen(&this_client_rect);
	//tmp_rect.OffsetRect(this_global_rect.left-this_client_rect.left,this_global_rect.top-this_client_rect.top);
	tmp_rect.right=this_client_rect.right-5;
	tmp_rect.bottom=this_client_rect.bottom-5;
	tmp_rect.top=240;
	tmp_rect.left=5;

	
	m_main_window.MoveWindow(tmp_rect,TRUE);
}

void CNetworkMonitorDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_main_window.MouseOff();
	CDialog::OnMouseMove(nFlags, point);
}

void CNetworkMonitorDialog::OnTimer(UINT nIDEvent)
{
	if(nIDEvent==1){
		CString tmp;
		tmp.Format("%d",mp_network_system->m_alive);
		m_alive.SetWindowText(tmp);

		tmp.Format("%d",mp_network_system->m_dead);
		m_dead.SetWindowText(tmp);

		tmp.Format("%d",mp_network_system->m_crashed);
		m_crashed.SetWindowText(tmp);

		tmp.Format("%.1f %%",mp_network_system->m_percent_alive);
		m_alive_percent.SetWindowText(tmp);

		tmp.Format("%.1f %%",mp_network_system->m_percent_not_crashed);
		m_not_crashed_percent.SetWindowText(tmp);
	}
	else if(nIDEvent==2){
		if(mp_network_system){
			mp_network_system->MarkOnTime();
		}
	}
	else if(nIDEvent==3){
		if(mp_network_system){
			mp_network_system->FillTreeWithDllInfo(m_dll_tree);
			mp_network_system->FillTreeWithProcessInfo(m_process_tree);
		}
	}


	CDialog::OnTimer(nIDEvent);
}

void CNetworkMonitorDialog::OnDestroy()
{
	CDialog::OnDestroy();

	this->KillTimer(1);
	this->KillTimer(2);
	this->KillTimer(3);
}
BOOL CNetworkMonitorDialog::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN){
		if(pMsg->wParam==VK_DOWN){
			m_main_window.OnAcceleratorDownKey();
			return TRUE; //the message has been translated, allow no further dispatching
		}
		else if(pMsg->wParam==VK_UP){
			m_main_window.OnAcceleratorUpKey();
			return TRUE; //the message has been translated, allow no further dispatching
		}	
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CNetworkMonitorDialog::OnGlobalRestartDead()
{
	if(IDYES==MessageBox("Are you sure you want to restart all dead or crashed racks?","VERIFY",MB_YESNO)){
		mp_network_system->OnRestartDeadRacks();
	}
}

void CNetworkMonitorDialog::OnSelectedRestartCrashed()
{
	mp_network_system->OnRestartCrashedSelectedNetwork(m_main_window.GetSelectedNetwork());
}

void CNetworkMonitorDialog::OnSelectedRestartDead()
{
	mp_network_system->OnRestartDeadSelectedNetwork(m_main_window.GetSelectedNetwork());
}

void CNetworkMonitorDialog::OnSelectedRestartAll()
{
	mp_network_system->OnRestartSelectedNetwork(m_main_window.GetSelectedNetwork());
}

void CNetworkMonitorDialog::OnSelectedRemoteCrashed()
{
	mp_network_system->OnRemoteCrashedSelectedNetwork(m_main_window.GetSelectedNetwork());
}

void CNetworkMonitorDialog::OnSelectedRemoteDead()
{
	mp_network_system->OnRemoteDeadSelectedNetwork(m_main_window.GetSelectedNetwork());
}
