// AresSupernodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AresSupernodeDlg.h"
#include "AresSupernodeSystem.h"
#include "AresSNSpoof.h"

// CAresSupernodeDlg dialog

IMPLEMENT_DYNAMIC(CAresSupernodeDlg, CDialog)
CAresSupernodeDlg::CAresSupernodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAresSupernodeDlg::IDD, pParent)
{
}

CAresSupernodeDlg::~CAresSupernodeDlg()
{
}

void CAresSupernodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONNECTIONLOG, m_connection_log);
	DDX_Control(pDX, IDC_QUERYLOG, m_query_log);
	DDX_Control(pDX, IDC_TOTALCONNECTIONS, m_total_connections);
	DDX_Control(pDX, IDC_FULLYCONNECTED, m_fully_connected);
	DDX_Control(pDX, IDC_KNOWNHOSTS, m_known_hosts);
	DDX_Control(pDX, IDC_LOADEDDECOYS, m_loaded_decoys);
	DDX_Control(pDX, IDC_UDP_HOSTS, m_udp_hosts);
	DDX_Control(pDX, IDC_LOADEDUSERNAMES, m_loaded_user_names);
}


BEGIN_MESSAGE_MAP(CAresSupernodeDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_TESTQUERY, OnTestQuery)
END_MESSAGE_MAP()


// CAresSupernodeDlg message handlers

void CAresSupernodeDlg::OnTimer(UINT nIDEvent)
{
	if(nIDEvent==1){
		AresSupernodeSystemRef ref;
		//ref.System()->LogToFile("CAresSupernodeDlg::OnTimer(1) BEGIN");
		ref.System()->UpdateTCP();
		//ref.System()->LogToFile("CAresSupernodeDlg::OnTimer(1) END");
	}
	else if(nIDEvent==2){
		AresSupernodeSystemRef ref;
		ref.System()->LogToFile("CAresSupernodeDlg::OnTimer(2) BEGIN");
		ref.System()->Update();
		ref.System()->LogToFile("CAresSupernodeDlg::OnTimer(2) END");
	}
	else if(nIDEvent==3){
		AresSupernodeSystemRef ref;
		ref.System()->LogToFile("CAresSupernodeDlg::OnTimer(3) BEGIN");
		UpdateGUI();
		ref.System()->LogToFile("CAresSupernodeDlg::OnTimer(3) END");
	}
	else if(nIDEvent==4){
		AresSupernodeSystemRef ref;
		//ref.System()->LogToFile("CAresSupernodeDlg::OnTimer(4) BEGIN");
		UpdateLogs();
		//ref.System()->LogToFile("CAresSupernodeDlg::OnTimer(4) END");
	}
	CDialog::OnTimer(nIDEvent);
}

BOOL CAresSupernodeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//this->SetTimer(1,100,NULL);  //MAKE FAST
	this->SetTimer(2,1000,NULL);
	this->SetTimer(3,3000,NULL);
	this->SetTimer(4,200,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAresSupernodeDlg::OnDestroy()
{
	CDialog::OnDestroy();

	this->KillTimer(1);
	this->KillTimer(2);
	this->KillTimer(3);
	this->KillTimer(4);
}

void CAresSupernodeDlg::UpdateGUI(void)
{
	AresSupernodeSystemRef ref;
	CString tmp;
	tmp.Format("%u",ref.System()->GetFullyConnected());
	m_fully_connected.SetWindowText(tmp);

	tmp.Format("%u",ref.System()->GetTotalConnections());
	m_total_connections.SetWindowText(tmp);

	tmp.Format("%u",ref.System()->GetKnownHosts());
	m_known_hosts.SetWindowText(tmp);

	tmp.Format("%u",ref.System()->GetLoadedDecoys());
	m_loaded_decoys.SetWindowText(tmp);

	tmp.Format("%u",ref.System()->GetUDPHostCount());
	m_udp_hosts.SetWindowText(tmp);

	tmp.Format("%u",ref.System()->GetHostCache()->GetLoadedUserNames());
	m_loaded_user_names.SetWindowText(tmp);
}

void CAresSupernodeDlg::UpdateLogs(void)
{
	AresSupernodeSystemRef ref;
	Vector v_tmp;

	//log any query info to the screen
	for(int i=0;i<2;i++){
		v_tmp.Clear();
		ref.System()->PopQueryLog(v_tmp);
		if(v_tmp.Size()>0){
			AresSNLogEntry* log_entry=(AresSNLogEntry*)v_tmp.Get(0);
			if(m_query_log.GetLineCount()>1000){
				//clear out oldest 3/4ths of the buffer
				CString tmp_str;  
				m_query_log.GetTextRange(3*m_query_log.GetTextLength()/4,m_query_log.GetTextLength(),tmp_str);
				m_query_log.SetWindowText(tmp_str);
			}
			m_query_log.SetSel(m_query_log.GetTextLength(),-1);
			m_query_log.ReplaceSel(log_entry->m_log.c_str());
		}
	}

	


	//log any connection info to the screen
	for(int i=0;i<2;i++){
		v_tmp.Clear();
		ref.System()->PopConnectionLog(v_tmp);
		if(v_tmp.Size()>0){
			AresSNLogEntry* log_entry=(AresSNLogEntry*)v_tmp.Get(0);
			if(m_connection_log.GetLineCount()>1000){
				//clear out oldest 3/4ths of the buffer
				CString tmp_str;  
				m_connection_log.GetTextRange(3*m_connection_log.GetTextLength()/4,m_connection_log.GetTextLength(),tmp_str);
				m_connection_log.SetWindowText(tmp_str);
			}
			m_connection_log.SetSel(m_connection_log.GetTextLength(),-1);
			m_connection_log.ReplaceSel(log_entry->m_log.c_str());
		}
	}
}

void CAresSupernodeDlg::OnTestQuery()
{
	vector<string> v_keywords;
	v_keywords.push_back("eminem");
	v_keywords.push_back("just");
	v_keywords.push_back("lose");
	v_keywords.push_back("it");
	Vector v_spoofs;
	AresSupernodeSystemRef ref;
	ref.System()->GetSpoofCache()->GetMatchingSpoofs(255,v_keywords,v_spoofs);

	v_spoofs.Sort(1);
	TRACE("\nFAKE QUERY HIT:\n");
	for(int i=0;i<(int)v_spoofs.Size();i++){
		AresSNSpoof* as=(AresSNSpoof*)v_spoofs.Get(i);
		TRACE("SPOOF[%d]: %u %s   %s   %s  %u\n",i,as->m_popularity,as->m_title.c_str(),as->m_artist.c_str(),as->m_filename.c_str(),as->m_size);
	}
	TRACE("END QUERY HIT!!\n\n");
}
