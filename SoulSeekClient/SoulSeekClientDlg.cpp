// SoulSeekClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SoulSeekClientDll.h"
#include "SoulSeekClientDlg.h"


// SoulSeekClientDlg dialog

IMPLEMENT_DYNAMIC(SoulSeekClientDlg, CDialog)
SoulSeekClientDlg::SoulSeekClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SoulSeekClientDlg::IDD, pParent)
{

	m_connectedParents = 0;
	m_totalParents = 0;
	m_parentModules = 0;
	m_peers = 0;
	m_peerModules = 0;
}

SoulSeekClientDlg::~SoulSeekClientDlg()
{
	if(pClientModTimer != NULL)
   {
	   KillTimer(pClientModTimer);
	   pClientModTimer = NULL;
   }
}

//
//
//
void SoulSeekClientDlg::InitParent(SoulSeekClientDll *parent)
{
	p_parent=parent;
}

void SoulSeekClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB2, m_mainTab);
	DDX_Control(pDX, IDC_RICHEDIT32, m_socket_left);
	DDX_Control(pDX, IDC_RICHEDIT33, m_socket_right);
}

BOOL SoulSeekClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_mainTab.InitDialogs();

	CRect rect;
	m_mainTab.GetClientRect( &rect );
	m_mainTab.SetMinTabWidth( rect.Width() - 2);

	m_mainTab.InsertItem( 0, " Program Log" );

	m_mainTab.ActivateTabDialogs();

	m_socket_left.SetBackgroundColor(FALSE, RGB(236,233, 216));
	m_socket_right.SetBackgroundColor(FALSE, RGB(236,233, 216));

	RefreshSocketInfo();

	return true;
}


BEGIN_MESSAGE_MAP(SoulSeekClientDlg, CDialog)
	ON_BN_CLICKED(IDSTART, OnBnClickedStart)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// SoulSeekClientDlg message handlers

void SoulSeekClientDlg::OnBnClickedStart()
{
	// TODO: Add your control notification handler code here
	if(m_Connected == false)
	{
		//Start Listener Thread
		WriteToLog(0,MAIN_SERVER_NUM,"Program started");
	}
	else
	{
		p_parent->m_connectionManager.CloseAllModules();
		WriteToLog(0,MAIN_SERVER_NUM,"Connection closed by user");
		m_connectedParents = 0;
		m_peers = 0;
		RefreshSocketInfo();

	}
}

void SoulSeekClientDlg::EnableLoginButton()
{
	//login
	WriteToLog(0,MAIN_SERVER_NUM,"Program Started");
}

// Writes to log box in the GUI
void SoulSeekClientDlg::WriteToLog(int level,int object,LPCTSTR strToWrite)
{
	m_mainTab.WriteToLog(level, object, strToWrite);
}

void SoulSeekClientDlg::DeleteClient(ClientModule* cmod)
{
	m_clientMods.push_back(cmod);
	if(pClientModTimer == NULL)
		pClientModTimer = SetTimer(0, 2*1000, 0);
}

void SoulSeekClientDlg::StopTimer() 
{

}


void SoulSeekClientDlg::OnTimer(UINT nIDEvent)
{
	if(pClientModTimer != NULL)
	{
		KillTimer(pClientModTimer);
		pClientModTimer = NULL;
	}
	while( !m_clientMods.empty() )
	{
		delete m_clientMods.back();
		m_clientMods.pop_back();
	}
}

void SoulSeekClientDlg::SetParentSocketData(int moreConnections, int moreParents, int moreModules, int morePeers, int morePM)
{
	m_connectedParents += moreConnections;
	m_totalParents += moreParents;
	m_parentModules += moreModules;
	m_peers += morePeers;
	m_peerModules += morePM;
	
	RefreshSocketInfo();
}
void SoulSeekClientDlg::RefreshSocketInfo()
{
	m_socket_left.SetWindowText("");
	m_socket_right.SetWindowText("");

	char tempString[10];

	itoa(m_connectedParents,tempString,10);
	AppendString("Parents Connected:",&m_socket_left, 0, 0, 0, false);
	AppendString(tempString,&m_socket_right, 0, 0, 0, false);
	AppendString(" / ",&m_socket_right, 0, 0, 0, false);
	itoa(m_totalParents,tempString,10);
	AppendString(tempString,&m_socket_right, 0, 0, 0, false);

	AppendString("\r\nParent Modules:",&m_socket_left, 0, 0, 0, false);
	AppendString("\r\n",&m_socket_right, 0, 0, 0, false);
	itoa(m_parentModules,tempString,10);
	AppendString(tempString,&m_socket_right, 0, 0, 0, false);

	AppendString("\r\n\r\nPeers Connected:",&m_socket_left, 0, 0, 0, false);
	AppendString("\r\n\r\n",&m_socket_right, 0, 0, 0, false);
	itoa(m_peers,tempString,10);
	AppendString(tempString,&m_socket_right, 0, 0, 0, false);

	AppendString("\r\nPeer Modules:",&m_socket_left, 0, 0, 0, false);
	AppendString("\r\n",&m_socket_right, 0, 0, 0, false);
	itoa(m_peerModules,tempString,10);
	AppendString(tempString,&m_socket_right, 0, 0, 0, false);
}
void SoulSeekClientDlg::AppendString(LPCTSTR tempString, CRichEditCtrl *rEBox, int red, int green, int blue, bool bold)
{
	rEBox->SetSel(-1,-1);  //if you wanted to append text to the end of the edit control
    CHARFORMAT cf;

    rEBox->GetSelectionCharFormat(cf);
    cf.dwMask |= CFM_COLOR;
    if(bold) cf.dwMask |= CFM_BOLD; 
	//cf.dwEffects = 0; 
	if(bold) cf.dwEffects |=CFE_BOLD;

	if(cf.dwEffects&CFE_AUTOCOLOR)//CFE_AUTOCOLOR=0x00000100
		cf.dwEffects^=CFE_AUTOCOLOR;

    cf.crTextColor = RGB(red,green,blue);                 //some color
    rEBox->SetSelectionCharFormat(cf);
    rEBox->ReplaceSel(tempString);
}

