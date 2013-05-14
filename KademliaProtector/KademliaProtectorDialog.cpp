// KademliaProtectorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "KademliaProtectorDialog.h"
#include "KademliaProtectorSystem.h"


// KademliaProtectorDialog dialog

IMPLEMENT_DYNAMIC(KademliaProtectorDialog, CDialog)
KademliaProtectorDialog::KademliaProtectorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(KademliaProtectorDialog::IDD, pParent)
{
}

KademliaProtectorDialog::~KademliaProtectorDialog()
{
}

void KademliaProtectorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NUMBERCONTACTS, m_number_contacts);
	DDX_Control(pDX, IDC_NUMBERNEWCONTACTS, m_number_new_contacts);
	DDX_Control(pDX, IDC_NUMBERFTCLIENTS, m_number_ft_clients);
	DDX_Control(pDX, IDC_NUMBERSWARMS, m_number_swarms);
	DDX_Control(pDX, IDC_CONTACTLOADSTATUS, m_contact_load_status);
}


BEGIN_MESSAGE_MAP(KademliaProtectorDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// KademliaProtectorDialog message handlers

BOOL KademliaProtectorDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	this->SetTimer(1,500,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void KademliaProtectorDialog::OnDestroy()
{
	CDialog::OnDestroy();

	this->KillTimer(1);
	// TODO: Add your message handler code here
}

void KademliaProtectorDialog::OnTimer(UINT nIDEvent)
{
	KademliaProtectorSystemReference ref;
	ref.System()->LogToFile("KademliaProtectorDialog::OnTimer(UINT nIDEvent) BEGIN");
	UpdateGUI();
	
	ref.System()->GuiClearTerminalFTClients();
	CDialog::OnTimer(nIDEvent);
	ref.System()->LogToFile("KademliaProtectorDialog::OnTimer(UINT nIDEvent) END");
}

void KademliaProtectorDialog::UpdateGUI(void)
{
	KademliaProtectorSystemReference ref;

	CString tmp;
	tmp.Format(("%u"),ref.System()->GetContactManager()->GetContactCount());
	m_number_contacts.SetWindowText(tmp);

	tmp.Format(("%u"),ref.System()->GetContactManager()->GetNewCachedContactCount());
	m_number_new_contacts.SetWindowText(tmp);

	tmp.Format(("%u"),ref.System()->GetFTClientCount());
	m_number_ft_clients.SetWindowText(tmp);

	tmp.Format(("%u"),ref.System()->GetSwarmCount());
	m_number_swarms.SetWindowText(tmp);

	m_contact_load_status.SetWindowText((const char*)ref.System()->GetContactManager()->GetContactLoadStatus());
}
