// CBTDecoyClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTDecoyClientDll.h"
#include "BTDecoyClientDlg.h"
#include "TorrentPiece.h"
#include "Iphlpapi.h"		// for GetAdaptersInfo (also need Iphlpapi.lib)
#include "Psapi.h"

#define WM_BTDECOYCLIENT_TIMER_PROCCOUNTER_UPDATE	WM_USER+1
#define WM_BTDECOYCLIENT_TIMER_TORRENT_UPDATE		WM_USER+2

IMPLEMENT_DYNAMIC(CBTDecoyClientDlg, CDialog)

BEGIN_MESSAGE_MAP(CBTDecoyClientDlg, CDialog)
	ON_WM_TIMER()
	ON_NOTIFY(HDN_ITEMDBLCLICK, 0, OnHdnItemdblclickTorrentsList)
END_MESSAGE_MAP()

//
//
//
CBTDecoyClientDlg::CBTDecoyClientDlg(CWnd* pParent)//=NULL)
: CDialog(CBTDecoyClientDlg::IDD, pParent), m_nMemLeft(0), m_nMaxLogEntries(0)
{
}

//
//
//
CBTDecoyClientDlg::~CBTDecoyClientDlg()
{
}

//
//
//
void CBTDecoyClientDlg::UpdateTorrents(const map<Hash20Byte, const Torrent *> *pmpTorrents)
{
	if( pmpTorrents != NULL )
	{
		char aHashBuf[40+1];
		char aLengthBuf[10+1];
		char aIDBuf[10+1];
		for( map<Hash20Byte, const Torrent *>::const_iterator torrentIter = pmpTorrents->begin();
			torrentIter != pmpTorrents->end(); torrentIter++ )
		{
			m_vTorrentFilename.push_back( torrentIter->second->GetFilename() );
			m_vTorrentHash.push_back( torrentIter->second->GetInfoHashPrintable( aHashBuf ) );
			m_vTorrentTotalLength.push_back( uitoa( torrentIter->second->GetTotalLength(), aLengthBuf ) );
			m_vTorrentID.push_back( uitoa( torrentIter->second->GetTorrentID(), aIDBuf ) );

			map<unsigned int, unsigned int> mTorrentPieces;
			map<unsigned int, unsigned int>::iterator intervalIter;
			for( set<TorrentPiece>::const_iterator pieceIter = torrentIter->second->m_sSeededPieces.begin();
				pieceIter != torrentIter->second->m_sSeededPieces.end(); pieceIter++ )
			{
				intervalIter = mTorrentPieces.find( pieceIter->m_nTimeInterval );
				if( intervalIter == mTorrentPieces.end() )
				{
					mTorrentPieces.insert( make_pair( pieceIter->m_nTimeInterval, 1 ) );
				}
				else
				{
					(intervalIter->second)++;
				}
			}
			m_vmTorrentPieces.push_back( mTorrentPieces );
			m_vTorrentTotalPieces.push_back( torrentIter->second->GetBitsInBitfield() );

			m_vTorrentActivationTime.push_back( torrentIter->second->GetActivationTime() );
		}
	}

	m_torrentsList.SetRedraw( false );
	m_torrentsList.DeleteAllItems();

	int nInsertPoint = 0;
	for( size_t i = 0; i < m_vTorrentFilename.size(); i++ )
	{
		nInsertPoint = m_torrentsList.InsertItem( 0, m_vTorrentFilename[i].c_str() );

		if( nInsertPoint == -1 )
			continue;

		m_torrentsList.SetItemText( nInsertPoint, 1, m_vTorrentHash[i].c_str() );
		m_torrentsList.SetItemText( nInsertPoint, 2, m_vTorrentTotalLength[i].c_str() );
		m_torrentsList.SetItemText( nInsertPoint, 3, m_vTorrentID[i].c_str() );

		int nPieces = 0;
		unsigned int nMinutes = (unsigned int)( CTime::GetCurrentTime() - m_vTorrentActivationTime[i] ).GetTotalMinutes();
		for( map<unsigned int, unsigned int>::const_iterator intervalIter = m_vmTorrentPieces[i].begin();
			intervalIter != m_vmTorrentPieces[i].end(); intervalIter++ )
		{
			if( nMinutes > intervalIter->first )
				nPieces += intervalIter->second;
		}

		char aBuf[16];
		sprintf( aBuf, "%0.02f", 100 * nPieces / (float)m_vTorrentTotalPieces[i] );
		m_torrentsList.SetItemText( nInsertPoint, 4, aBuf );
	}
	UpdateData(true);

	m_torrentsList.SetRedraw( true );
	m_torrentsList.Invalidate();

	UpdateWindow();
}

//
//
//
void CBTDecoyClientDlg::InitPerformanceCounters(void)
{
	char iai_buf[1024];
	DWORD iai_buf_len = 1024;
	IP_ADAPTER_INFO *iai = (IP_ADAPTER_INFO *)iai_buf;

	GetAdaptersInfo( iai, &iai_buf_len );

	// Remove (,) and / from the description of the interface
	while(strchr(iai->Description,'(')!=NULL)
	{
		*strchr(iai->Description,'(')='[';
	}
	while(strchr(iai->Description,')')!=NULL)
	{
		*strchr(iai->Description,')')=']';
	}
	while(strchr(iai->Description,'/')!=NULL)
	{
		*strchr(iai->Description,'/')='_';
	}

	string keynames[3];
	keynames[0] = "\\Processor(0)\\% Processor Time";
	keynames[1] = "\\Network Interface(";
	keynames[1] += iai->Description;
	keynames[1] += ")\\Bytes Total/sec";
	keynames[2] = "\\Memory\\Available MBytes";

    m_pdh = 0;

    // Create the pdh query
    if( PdhOpenQuery( NULL, 0, &m_pdh ) )
		return;

    // ADD A COUNTER TO THE QUERY
	for( int i = 0; i < 3; i++ )
	{
		if( PdhAddCounter( m_pdh, keynames[i].c_str(), NULL, &m_pdh_counters[i] ) )
			return;
	}
}

//
//
//
void CBTDecoyClientDlg::LogMsg(const char *pStr)
{
	if( pStr == NULL )
		return;

	while( m_logList.GetItemCount() >= m_nMaxLogEntries )
		m_logList.DeleteItem( 0 );

//	TRACE( "%s\n", pStr );
	m_logList.InsertItem( m_logList.GetItemCount(), CTime::GetCurrentTime().Format( "%H:%M:%S: " ) + pStr );
}

//
//
//
BOOL CBTDecoyClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	m_logList.GetClientRect( rect );
	m_logList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_logList.InsertColumn( 0, "Entry", LVCFMT_LEFT, rect.Width() );
	CRect itemRect;
	m_logList.InsertItem( 0, "" );
	m_logList.GetItemRect( 0, itemRect, 0 );
	m_nMaxLogEntries = rect.Height() / itemRect.Height();
	m_logList.DeleteItem( 0 );

	m_torrentsList.GetClientRect( rect );
	m_torrentsList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_torrentsList.InsertColumn( 0, "Filename", LVCFMT_LEFT, 300 );
	m_torrentsList.InsertColumn( 1, "InfoHash", LVCFMT_LEFT, 275 );
	m_torrentsList.InsertColumn( 2, "Size", LVCFMT_RIGHT, 75 );
	m_torrentsList.InsertColumn( 3, "ID", LVCFMT_RIGHT, 50 );
	m_torrentsList.InsertColumn( 4, "% Seeded", LVCFMT_RIGHT, rect.Width() - (700 + GetSystemMetrics(SM_CXVSCROLL)) );

	InitPerformanceCounters();
	SetTimer( WM_BTDECOYCLIENT_TIMER_PROCCOUNTER_UPDATE, 1000, NULL );
	SetTimer( WM_BTDECOYCLIENT_TIMER_TORRENT_UPDATE, 60 * 1000, NULL );

	return TRUE;
}

//
//
//
void CBTDecoyClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG_LIST, m_logList);
	DDX_Control(pDX, IDC_TORRENTS_LIST, m_torrentsList);
	DDX_Control(pDX, IDC_COMPLETE_CHECK, m_completeCheck);
	DDX_Control(pDX, IDC_MODULES_STATIC, m_modulesStatic);
	DDX_Control(pDX, IDC_CLIENTS_STATIC, m_clientsStatic);
}

//
//
//
void CBTDecoyClientDlg::OnHdnItemdblclickTorrentsList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	// TODO: Add your control notification handler code here

	*pResult = 0;
}

//
//
//
void CBTDecoyClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch( nIDEvent )
	{
	case WM_BTDECOYCLIENT_TIMER_PROCCOUNTER_UPDATE:
		{
			// Collect the pdh query data
			if( !PdhCollectQueryData( m_pdh ) )
			{
				PDH_FMT_COUNTERVALUE value;

				// Check the memory counter
				if( !PdhGetFormattedCounterValue( m_pdh_counters[2], PDH_FMT_LONG, 0, &value ) )
				{
					m_nMemLeft = value.longValue;
				}

				// Measure the processes mem usage
				PROCESS_MEMORY_COUNTERS pmc;
				pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
				HANDLE hHandle = NULL;
				DuplicateHandle( GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &hHandle, 0,
					FALSE, DUPLICATE_SAME_ACCESS );
				GetProcessMemoryInfo( hHandle, &pmc, sizeof(PROCESS_MEMORY_COUNTERS) );
				CloseHandle( hHandle );
				m_nMemUse = (UINT)(pmc.WorkingSetSize >> 20);
			}
		}
		break;
	case WM_BTDECOYCLIENT_TIMER_TORRENT_UPDATE:
		{
			UpdateTorrents( NULL );

			char aBuf[32];

			sprintf( aBuf, "Modules: %u", m_pDll->GetModuleCount() );
			m_modulesStatic.SetWindowText( aBuf );

			sprintf( aBuf, "Clients: %u", m_pDll->GetClientCount() );
			m_clientsStatic.SetWindowText( aBuf );
		}
		break;
	}

	__super::OnTimer( nIDEvent );
}

