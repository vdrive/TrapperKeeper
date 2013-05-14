// ConnectedPeersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConnectedPeersDlg.h"
#include "Peer.h"


// CConnectedPeersDlg dialog

IMPLEMENT_DYNAMIC(CConnectedPeersDlg, CDialog)
CConnectedPeersDlg::CConnectedPeersDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectedPeersDlg::IDD, pParent)
{
}

//
//
//
CConnectedPeersDlg::~CConnectedPeersDlg()
{
}

//
//
//
void CConnectedPeersDlg::ConnectedPeers(const set<Peer> *pConnectedPeers)
{
	if( pConnectedPeers != NULL )
	{
		int nIndex;
		IPPort ip;
		m_peerList.DeleteAllItems();
		
		for( set<Peer>::const_iterator iter = pConnectedPeers->begin(); iter != pConnectedPeers->end(); iter++ )
		{
			ip.SetIP( iter->GetIP() );
			ip.SetPort( iter->GetPort() );

			nIndex = m_peerList.InsertItem( 0, ip.ToString().c_str() );

			if( nIndex == -1 )
			{
				continue;
			}

			m_peerList.SetItemText( nIndex, 1, ( iter->IsComplete() ? "true" : "false" ) );
			m_peerList.SetItemText( nIndex, 2, ( iter->IsMD() ? "true" : "false" ) );

			static char aListBuf[32];
			CTimeSpan timeSpan( iter->SecondsSinceLastSeen() );
			m_peerList.SetItemText( nIndex, 3, timeSpan.Format( "%D days, %H:%M:%S ago" ) );
		}

		delete pConnectedPeers;
	}
}

//
//
//
void CConnectedPeersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONNECTED_PEERS_LIST, m_peerList);
}

BEGIN_MESSAGE_MAP(CConnectedPeersDlg, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

//
//
//
BOOL CConnectedPeersDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Determine the width of the log window to set the only column to its maximum width
	CRect logRect;
	m_peerList.GetClientRect( &logRect );

	m_peerList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_peerList.InsertColumn( 0, "IP : Port", LVCFMT_LEFT, 200 );
	m_peerList.InsertColumn( 1, "Complete", LVCFMT_LEFT, 100 );
	m_peerList.InsertColumn( 2, "MediaDefender", LVCFMT_LEFT, 100 );
	m_peerList.InsertColumn( 4, "Last Active", LVCFMT_LEFT, logRect.Width() - ( 400 + GetSystemMetrics(SM_CXVSCROLL) ) );

	// return TRUE  unless you set the focus to a control
	return TRUE;  
}

//
//
//
void CConnectedPeersDlg::OnClose()
{
	ShowWindow(SW_HIDE);
}
