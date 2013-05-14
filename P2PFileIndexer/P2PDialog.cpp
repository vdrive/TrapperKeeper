// P2PDialog.cpp : implementation file
//

#include "stdafx.h"
#include "P2PDialog.h"
#include "..\TKFileTransfer\TKFileTransferInterface.h"
#include "..\nameserver\NameServerInterface.h"


// P2PDialog dialog

IMPLEMENT_DYNAMIC(P2PDialog, CDialog)
P2PDialog::P2PDialog(CWnd* pParent /*=NULL*/)
	: CDialog(P2PDialog::IDD, pParent)
{
	m_ip_index=0;
	mp_dll=NULL;
}

P2PDialog::~P2PDialog()
{
}

void P2PDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(P2PDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// P2PDialog message handlers

BOOL P2PDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_com.Register(mp_dll,62); //we want to talk to file transfer services everywhere, thus we will use the file transfer com's opcode.
	this->SetTimer(1,10000,NULL);  //every 10 seconds send more server address notifications
	m_com.StartThread();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void P2PDialog::OnTimer(UINT nIDEvent)
{
	static int t_count=0;
	if(m_ip_index>=(int)mv_client_ips.size()){
		m_ip_index=0;
	}

	TKFileTransferInterface::IPInterface message;
	message.m_type=message.P2PSERVERADDRESS;
	byte buf[1024];
	int len=message.WriteToBuffer(buf);

	//ping a few more of our clients, letting them know our address
	for(int count=0;m_ip_index<(int)mv_client_ips.size() && count<5;m_ip_index++,count++){
		//TRACE("P2PIndexingService P2PDialog::OnTimer() Broadcasting server init to %s.\n",mv_client_ips[m_ip_index].c_str());
		m_com.SendReliableData((char*)mv_client_ips[m_ip_index].c_str(),buf,len);
	}

	if(((t_count++)%5)==0){  //update the com and file index now and then
		m_com.Update();
	}

	if(mv_client_ips.size()==0 || (rand()%30)==0){  
		UpdateIPs();  //update our ips initialy and every now and then
	}


	CDialog::OnTimer(nIDEvent);
}

void P2PDialog::OnDestroy()
{
	m_com.StopThread();
	CDialog::OnDestroy();

	KillTimer(1);
	
}

void P2PDialog::SetDLL(Dll* p_dll)
{
	mp_dll=p_dll;
	m_com.Register(mp_dll,62);
}

//call to update the list of ips of our clients
void P2PDialog::UpdateIPs(void)
{
	TRACE("P2PIndexingService P2PDialog::UpdateIPs().\n");
	NameServerInterface ns;
	mv_client_ips.clear();
	CString tag="P2P";
	ns.RequestIP(tag,mv_client_ips);
}
