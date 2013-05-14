// MetaMachineSpooferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MetaMachineSpooferDlg.h"
//#include "MetaSpooferTCP.h"
#include "..\tkcom\Timer.h"
#include "spoofingsystem.h"


// CMetaMachineSpooferDlg dialog

IMPLEMENT_DYNAMIC(CMetaMachineSpooferDlg, CDialog)
CMetaMachineSpooferDlg::CMetaMachineSpooferDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMetaMachineSpooferDlg::IDD, pParent)
{
	m_connection_count=0;
}

CMetaMachineSpooferDlg::~CMetaMachineSpooferDlg()
{
}

void CMetaMachineSpooferDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONNECTIONSPERHOUR, m_control_connections_per_hour);
	DDX_Control(pDX, IDC_CONNECTEDSERVERS, m_connected_servers);
	DDX_Control(pDX, IDC_DISCONNECTEDSERVERS, m_down_servers);
	DDX_Control(pDX, IDC_LOG, m_log);
	DDX_Control(pDX, IDC_LOG2, m_data_log);
	DDX_Control(pDX, IDC_CLIENT_CONNECTIONS, m_client_connection_count);
	DDX_Control(pDX, IDC_ACTIVEINTERDICTIONCONNECTIONS, m_active_interdiction_connection_count);
	DDX_Control(pDX, IDC_INACTIVEINTERDICTIONCONNECTIONS, m_inactive_interdiction_connection_count);
}


BEGIN_MESSAGE_MAP(CMetaMachineSpooferDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CMetaMachineSpooferDlg message handlers

BOOL CMetaMachineSpooferDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	this->SetTimer(1,400,NULL);
	m_control_connections_per_hour.SetWindowText("Waiting");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMetaMachineSpooferDlg::OnTimer(UINT nIDEvent)
{
	static bool b_in_timer=false;
	static Timer last_connection_measurement;
	static Timer last_server_update;
	if(b_in_timer)
		return;
	b_in_timer=true;

	_ASSERTE( _CrtCheckMemory( ) );

//	SpooferTCPReference tcpref;
//	tcpref.TCP()->Update();

	MetaSpooferReference sysref;
	sysref.System()->Update();

	if(last_connection_measurement.HasTimedOut(60*60)){
		last_connection_measurement.Refresh();
		CString txt;
		txt.Format("%d",m_connection_count);
		m_control_connections_per_hour.SetWindowText(txt);
		m_connection_count=0;
	}

	if(last_server_update.HasTimedOut(45)){
		sysref.System()->LogToFile("CMetaMachineSpooferDlg::OnTimer() - Server Status Update STARTED.\r\n");
		vector <string> v_up;
		vector <string> v_down;

		sysref.System()->GetConnectedServers(v_up);
		sysref.System()->GetDisconnectedServers(v_down);
		m_connected_servers.ResetContent();
		m_down_servers.ResetContent();
		for(int i=0;i<(int)v_up.size();i++){
			m_connected_servers.AddString(v_up[i].c_str());
		}
		for(int i=0;i<(int)v_down.size();i++){
			m_down_servers.AddString(v_down[i].c_str());
		}
		last_server_update.Refresh();

		CString tmp;
		tmp.Format("%d",sysref.System()->GetInactiveConnectionCount());
		m_inactive_interdiction_connection_count.SetWindowText(tmp);
		tmp.Format("%d",sysref.System()->GetActiveConnectionCount());
		m_active_interdiction_connection_count.SetWindowText(tmp);
		tmp.Format("%d",sysref.System()->GetClientConnectionCount());
		m_client_connection_count.SetWindowText(tmp);
		sysref.System()->LogToFile("CMetaMachineSpooferDlg::OnTimer() - Server Status Update FINISHED.\r\n");
	}

	if(mv_needed_logs.Size()>0){
		sysref.System()->LogToFile("CMetaMachineSpooferDlg::OnTimer() - Log Status Update STARTED.\r\n");
		if(m_log.GetLineCount()>3000){
			m_log.SetWindowText("");
		}
		m_log.SetSel(m_log.GetTextLength(),-1);
		m_log.ReplaceSel(((LogMsg*)mv_needed_logs.Get(0))->GetMsg());
		mv_needed_logs.Remove(0);
		sysref.System()->LogToFile("CMetaMachineSpooferDlg::OnTimer() - Log Status Update FINISHED.\r\n");
	}

	if(mv_needed_data_logs.Size()>0){
		sysref.System()->LogToFile("CMetaMachineSpooferDlg::OnTimer() - Data Log Status Update STARTED.\r\n");
		if(m_data_log.GetLineCount()>3000){
			m_data_log.SetWindowText("");
		}
		m_data_log.SetSel(m_data_log.GetTextLength(),-1);
		m_data_log.ReplaceSel(((LogMsg*)mv_needed_data_logs.Get(0))->GetMsg());
		mv_needed_data_logs.Remove(0);
		sysref.System()->LogToFile("CMetaMachineSpooferDlg::OnTimer() - Data Log Status Update FINISHED.\r\n");
	}

	CDialog::OnTimer(nIDEvent);
	b_in_timer=false;
	_ASSERTE( _CrtCheckMemory( ) );
}

void CMetaMachineSpooferDlg::OnDestroy()
{
	CDialog::OnDestroy();

	this->KillTimer(1);
}

void CMetaMachineSpooferDlg::GotConnection(void)
{
	m_connection_count++;
}

void CMetaMachineSpooferDlg::Log(const char* source_ip,const char* data)
{
	/*
	MetaSpooferReference sysref;
	sysref.System()->LogToFile("CMetaMachineSpooferDlg::Log() - STARTED.\r\n");
	CString output;
	CTime cur_time=CTime::GetCurrentTime();
	output.Format("%s %s - %s\r\n",source_ip,cur_time.Format("%H:%M"),data);
	mv_needed_logs.Add(new LogMsg(output));
	sysref.System()->LogToFile("CMetaMachineSpooferDlg::Log() - FINISHED.\r\n");
	*/
}

void CMetaMachineSpooferDlg::LogData(const char* source_ip, Buffer2000* buffer)
{
	/*
	MetaSpooferReference sysref;
	sysref.System()->LogToFile("CMetaMachineSpooferDlg::LogData() - STARTED.\r\n");
	const byte* p_buf=buffer->GetBufferPtr();
	int buf_len=buffer->GetLength();
	
	CString output;
	CTime cur_time=CTime::GetCurrentTime();
	output.Format("%s %s - ",source_ip,cur_time.Format("%H:%M"));
	for(int i=0;i<buf_len;i++){
		if(isalnum(p_buf[i]) || p_buf[i]==' '){
			output+=(char)p_buf[i];
		}
		if(p_buf[i]=='\n')
			output+="\r\n       ";  //indent after carriage return for logging readability so we can associate logs with one particular message visually
	}
	output+="\r\n";

	mv_needed_data_logs.Add(new LogMsg(output));
	sysref.System()->LogToFile("CMetaMachineSpooferDlg::LogData() - FINISHED.\r\n");
	*/
}
