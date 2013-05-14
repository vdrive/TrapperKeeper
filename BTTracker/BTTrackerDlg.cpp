//
// BTTrackerDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "BTTrackerDlg.h"
#include "BTTrackerDll.h"
#include "DllInterface.h"
#include "TorrentListUpdate.h"
#include "Peer.h"

//
// CBTTrackerDlg dialog
//

IMPLEMENT_DYNAMIC(CBTTrackerDlg, CDialog)
CBTTrackerDlg::CBTTrackerDlg(CWnd* pParent /*=NULL*/)
: CDialog(CBTTrackerDlg::IDD, pParent), m_pDll(NULL), m_nMaxLogEntries(0)
{
}

CBTTrackerDlg::~CBTTrackerDlg()
{
}

void CBTTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_torrentLogList);
	DDX_Control(pDX, IDC_TORRENT_INDEX_LIST, m_torrentIndexList);
	DDX_Control(pDX, IDC_CLIENTS_STATIC, m_clientsText);
	DDX_Control(pDX, IDC_MODULES_STATIC, m_modulesText);
}

BEGIN_MESSAGE_MAP(CBTTrackerDlg, CDialog)
	ON_BN_CLICKED(IDC_ADDINFOHASH_BUTTON, OnBnClickedAddinfohashButton)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_TORRENT_INDEX_LIST, OnNMDblclkTorrentIndexList)
	ON_BN_CLICKED(IDC_STALE_CHECK_BUTTON, OnBnClickedStaleCheckButton)
END_MESSAGE_MAP()

//
//
//
BOOL CBTTrackerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Determine the width of the log window to set the only column to its maximum width
	CRect logRect;
	m_torrentLogList.GetClientRect( &logRect );

	m_torrentLogList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_torrentLogList.InsertColumn( 0, "Entry", LVCFMT_LEFT, logRect.Width() );

	// Determine the height of the log window to set the maximum number of entries (without scrolling)
	CRect itemRect;
	m_torrentLogList.InsertItem( 0, "" );
	m_torrentLogList.GetItemRect( 0, &itemRect, LVIR_LABEL );
	m_nMaxLogEntries = logRect.Height() / itemRect.Height();
	m_torrentLogList.DeleteItem( 0 );

	m_torrentIndexList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_torrentIndexList.InsertColumn( 0, "Hash", LVCFMT_LEFT, 275 );
	m_torrentIndexList.InsertColumn( 1, "Seeds (R/I)", LVCFMT_LEFT, 100 );
	m_torrentIndexList.InsertColumn( 2, "Peers (R/I)", LVCFMT_LEFT, 100 );
	m_torrentIndexList.InsertColumn( 3, "DLs (R/I)", LVCFMT_LEFT, 100 );
	m_torrentIndexList.InsertColumn( 4, "Torrent Name", LVCFMT_LEFT, logRect.Width() - ( 675 + GetSystemMetrics(SM_CXVSCROLL) ) );
	m_torrentIndexList.InsertColumn( 5, "Last Active", LVCFMT_LEFT, 100 );

	// Create the child window
	m_addInfohashDlg.Create( IDD_ADDTORRENT_DIALOG, CWnd::GetDesktopWindow() );
	m_addInfohashDlg.Init( this );

	m_connectedPeersDlg.Create( IDD_CONNECTED_PEERS_DIALOG, CWnd::GetDesktopWindow() );

	// return TRUE  unless you set the focus to a control
	return TRUE;  
}

//
//
//
void CBTTrackerDlg::OnClose()
{
	ShowWindow(SW_HIDE);
	
	m_addInfohashDlg.ShowWindow(SW_HIDE);
	m_connectedPeersDlg.ShowWindow(SW_HIDE);
}

//
//
//
void CBTTrackerDlg::UpdateTorrentList(TorrentListUpdate *pTorrentListUpdate)
{
	if( pTorrentListUpdate == NULL )
		return;

	LVFINDINFO info;
	info.flags = LVFI_STRING;
	info.psz = pTorrentListUpdate->GetInfoHash();
	int nIndex = m_torrentIndexList.FindItem( &info );

	if( pTorrentListUpdate->IsRemove() )
	{
		if( nIndex != -1 )
		{
			m_torrentIndexList.DeleteItem( nIndex );
		}
	}
	else
	{
		if( nIndex == -1 )
		{
			// create it
			nIndex = m_torrentIndexList.InsertItem( 0, pTorrentListUpdate->GetInfoHash() );

			if( nIndex == -1 )
			{
				delete pTorrentListUpdate;
				return;
			}

			m_torrentIndexList.SetItemText( nIndex, 4, pTorrentListUpdate->GetTorrentName().c_str() );
		}

		static char aListBuf[128];

		sprintf( aListBuf, "%d / %d", pTorrentListUpdate->GetSeedsReal(), pTorrentListUpdate->GetSeedsInflated() );
		m_torrentIndexList.SetItemText( nIndex, 1, aListBuf );

		sprintf( aListBuf, "%d / %d", pTorrentListUpdate->GetPeersReal(), pTorrentListUpdate->GetPeersInflated() );
		m_torrentIndexList.SetItemText( nIndex, 2, aListBuf );

		sprintf( aListBuf, "%d / %d", pTorrentListUpdate->GetCompletedReal(), pTorrentListUpdate->GetCompletedInflated() );
		m_torrentIndexList.SetItemText( nIndex, 3, aListBuf );

		m_torrentIndexList.SetItemText( nIndex, 5, CTime::GetCurrentTime().Format( "%H:%M:%S" ) );
	}

	delete pTorrentListUpdate;
}

//
//
//
void CBTTrackerDlg::LogEntry(const char *pStr)
{
	if( pStr == NULL )
		return;

	TRACE( "%s\n", pStr );

	m_torrentLogList.InsertItem( m_torrentLogList.GetItemCount(),
		CTime::GetCurrentTime().Format( "%H:%M:%S: " ) + pStr );

	while( m_torrentLogList.GetItemCount() > m_nMaxLogEntries )
		m_torrentLogList.DeleteItem( 0 );
}

//
//
//
void CBTTrackerDlg::AddTorrent(Torrent *pTorrent)
{
	m_pDll->AddTorrent( pTorrent );
}

//
//
//
void CBTTrackerDlg::ConnectedPeers(const set<Peer> *pConnectedPeers)
{
	if( pConnectedPeers != NULL )
	{
		m_connectedPeersDlg.ShowWindow( SW_SHOW );
		m_connectedPeersDlg.BringWindowToTop();

		m_connectedPeersDlg.ConnectedPeers( pConnectedPeers );
	}
}

void CBTTrackerDlg::SetCounts(size_t nMods, size_t nClients)
{
	char aBuf[32];

	sprintf( aBuf, "Modules: %u", nMods );
	m_modulesText.SetWindowText( aBuf );

	sprintf( aBuf, "Clients: %u", nClients );
	m_clientsText.SetWindowText( aBuf );
}

//
//
//
void CBTTrackerDlg::OnBnClickedAddinfohashButton()
{
	m_addInfohashDlg.ShowWindow( SW_SHOW );
	m_addInfohashDlg.BringWindowToTop();
}

//
//
//
void CBTTrackerDlg::OnNMDblclkTorrentIndexList(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nIndex = m_torrentIndexList.GetSelectionMark();
	if( nIndex != -1 )
	{
		char aBuf[41];
		m_torrentIndexList.GetItemText( nIndex, 0, aBuf, 41 );
        
		Hash20Byte *pHash = new Hash20Byte;
		pHash->FromHexString( aBuf );

		m_pDll->GetConnectedPeers( pHash );
	}

	*pResult = 0;
}

//
//
//
void CBTTrackerDlg::OnBnClickedStaleCheckButton()
{
	m_pDll->CheckForStaleIPs();
}
