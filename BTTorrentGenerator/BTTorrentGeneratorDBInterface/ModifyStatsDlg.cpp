// ModifyStatsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTTorrentGeneratorDBInterface.h"
#include "ModifyStatsDlg.h"
#include "TorrentEntry.h"
#include "..\TinySQL.h"

#define WM_GET_TORRENTS_RESULT		WM_USER+1

UINT GetTorrentsThreadProc(LPVOID pParam);

// CModifyStatsDlg dialog

IMPLEMENT_DYNAMIC(CModifyStatsDlg, CDialog)
CModifyStatsDlg::CModifyStatsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyStatsDlg::IDD, pParent), m_pvTorrents(NULL)
	, m_bEditing(false), m_nEditSelection(-1)
{
}

CModifyStatsDlg::~CModifyStatsDlg()
{
	if( m_pvTorrents != NULL )
	{
		delete m_pvTorrents;
		m_pvTorrents = NULL;
	}
}

//
//
//
BOOL CModifyStatsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	m_torrentsList.GetWindowRect( &rect );
	int nWidth = rect.Width()-20;

	m_torrentsList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_torrentsList.InsertColumn( 0, "Filename", LVCFMT_LEFT, nWidth * 7 / 10 );
	m_torrentsList.InsertColumn( 1, "Hash", LVCFMT_LEFT, nWidth * 3 / 10 );

	GetDlgItem( IDC_BASE_SEEDS_EDIT )->EnableWindow( false );
	GetDlgItem( IDC_BASE_PEERS_EDIT )->EnableWindow( false );
	GetDlgItem( IDC_BASE_COMPLETES_EDIT )->EnableWindow( false );
	GetDlgItem( IDC_SEED_MULTIPLIER_EDIT )->EnableWindow( false );
	GetDlgItem( IDC_PEERS_MULTIPLIER_EDIT )->EnableWindow( false );
	GetDlgItem( IDC_COMPLETE_MULTIPLIER_EDIT )->EnableWindow( false );

	GetDlgItem( IDC_COMMENT_EDIT )->EnableWindow( false );

	GetDlgItem( IDC_ACTIVE_CHECK )->EnableWindow( false );
	GetDlgItem( IDC_GENERATE_CHECK )->EnableWindow( false );

	GetDlgItem( IDC_COMMIT_BUTTON )->EnableWindow( false );
	GetDlgItem( IDC_CANCEL_BUTTON )->EnableWindow( false );

	AfxBeginThread( GetTorrentsThreadProc, (LPVOID)GetSafeHwnd(), THREAD_PRIORITY_BELOW_NORMAL );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//
//
//
void CModifyStatsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TORRENT_LIST, m_torrentsList);
}

//
//
//
LRESULT CModifyStatsDlg::GetTorrentsResult(WPARAM wparam, LPARAM lparam)
{
	if( wparam == NULL )
		return -1;

	m_torrentsList.DeleteAllItems();
	m_torrentsList.SetSelectionMark( -1 );

	GetDlgItem( IDC_FILENAME_STATIC )->SetWindowText( "Name: <NONE>" );
	GetDlgItem( IDC_HASH_STATIC )->SetWindowText( "Hash: <NONE>" );
	GetDlgItem( IDC_LENGTH_STATIC )->SetWindowText( "0" );

	GetDlgItem( IDC_BASE_SEEDS_EDIT )->SetWindowText( "0" );
	GetDlgItem( IDC_BASE_PEERS_EDIT )->SetWindowText( "0" );
	GetDlgItem( IDC_BASE_COMPLETES_EDIT )->SetWindowText( "0" );
	GetDlgItem( IDC_SEED_MULTIPLIER_EDIT )->SetWindowText( "0" );
	GetDlgItem( IDC_PEERS_MULTIPLIER_EDIT )->SetWindowText( "0" );
	GetDlgItem( IDC_COMPLETE_MULTIPLIER_EDIT )->SetWindowText( "0" );

	GetDlgItem( IDC_COMMENT_EDIT )->SetWindowText( "" );

	GetDlgItem( IDC_CREATION_STATIC )->SetWindowText( "Creation Date: <NONE>" );
	GetDlgItem( IDC_ACTIVATION_STATIC )->SetWindowText( "Activation Date: <NONE>" );

	CheckDlgButton(IDC_ACTIVE_CHECK, BST_UNCHECKED);
	CheckDlgButton(IDC_GENERATE_CHECK, BST_UNCHECKED);
	CheckDlgButton(IDC_INFLATE_CHECK, BST_UNCHECKED);

	GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "" );

	if( m_pvTorrents != NULL )
		delete m_pvTorrents;
	m_pvTorrents = (vector<TorrentEntry> *)wparam;

	for( size_t i = 0; i < m_pvTorrents->size(); i++ )
	{
		m_torrentsList.InsertItem( (int)i, m_pvTorrents->at( i ).m_filename.c_str() );
		m_torrentsList.SetItemText( (int)i, 1, m_pvTorrents->at( i ).m_hash.c_str() );
	}

	return 0;
}

//
//
//
void CModifyStatsDlg::OnNMClickTorrentList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if( !m_bEditing )
	{
		char aBuf[512];
		int nSelection = m_torrentsList.GetSelectionMark();

		if( nSelection == -1 )
			return;

		sprintf( aBuf, "Name: %s", m_pvTorrents->at( nSelection ).m_filename.c_str() );
		GetDlgItem( IDC_FILENAME_STATIC )->SetWindowText( aBuf );

		sprintf( aBuf, "Hash: %s", m_pvTorrents->at( nSelection ).m_hash.c_str() );
		GetDlgItem( IDC_HASH_STATIC )->SetWindowText( aBuf );

		{
			sprintf( aBuf, "%u", m_pvTorrents->at( nSelection ).m_nLength );
			CString lengthBuf = aBuf;
			switch( lengthBuf.GetLength() )
			{
			case 10:
				{
					lengthBuf.Insert( lengthBuf.GetLength() - 9, ',' );
					lengthBuf.Insert( lengthBuf.GetLength() - 6, ',' );
					lengthBuf.Insert( lengthBuf.GetLength() - 3, ',' );
				}
				break;
			case 9:
			case 8:
			case 7:
				{
					lengthBuf.Insert( lengthBuf.GetLength() - 6, ',' );
					lengthBuf.Insert( lengthBuf.GetLength() - 3, ',' );
				}
				break;
			case 6:
			case 5:
			case 4:
				{
					lengthBuf.Insert( lengthBuf.GetLength() - 3, ',' );
				}
				break;
			}

			sprintf( aBuf, "Length: %s", lengthBuf );
			GetDlgItem( IDC_LENGTH_STATIC )->SetWindowText( aBuf );
		}

		sprintf( aBuf, "%u", m_pvTorrents->at( nSelection ).m_nBaseSeeds );
		GetDlgItem( IDC_BASE_SEEDS_EDIT )->SetWindowText( aBuf );

		sprintf( aBuf, "%u", m_pvTorrents->at( nSelection ).m_nBasePeers );
		GetDlgItem( IDC_BASE_PEERS_EDIT )->SetWindowText( aBuf );

		sprintf( aBuf, "%u", m_pvTorrents->at( nSelection ).m_nBaseCompletes );
		GetDlgItem( IDC_BASE_COMPLETES_EDIT )->SetWindowText( aBuf );

		sprintf( aBuf, "%u", m_pvTorrents->at( nSelection ).m_nSeedMultiplier );
		GetDlgItem( IDC_SEED_MULTIPLIER_EDIT )->SetWindowText( aBuf );

		sprintf( aBuf, "%u", m_pvTorrents->at( nSelection ).m_nPeerMultiplier );
		GetDlgItem( IDC_PEERS_MULTIPLIER_EDIT )->SetWindowText( aBuf );

		sprintf( aBuf, "%u", m_pvTorrents->at( nSelection ).m_nCompleteMultiplier );
		GetDlgItem( IDC_COMPLETE_MULTIPLIER_EDIT )->SetWindowText( aBuf );

		GetDlgItem( IDC_COMMENT_EDIT )->SetWindowText( m_pvTorrents->at( nSelection ).m_comment.c_str() );

		sprintf( aBuf, "Creation Date: %s", m_pvTorrents->at( nSelection ).m_aCreationDate );
		GetDlgItem( IDC_CREATION_STATIC )->SetWindowText( aBuf );

		sprintf( aBuf, "Activation Date: %s", m_pvTorrents->at( nSelection ).m_aActivationDate );
		GetDlgItem( IDC_ACTIVATION_STATIC )->SetWindowText( aBuf );

		if( m_pvTorrents->at( nSelection ).m_bActive )
			CheckDlgButton(IDC_ACTIVE_CHECK, BST_CHECKED);
		else
			CheckDlgButton(IDC_ACTIVE_CHECK, BST_UNCHECKED);

		if( m_pvTorrents->at( nSelection ).m_bGenerate )
			CheckDlgButton(IDC_GENERATE_CHECK, BST_CHECKED);
		else
			CheckDlgButton(IDC_GENERATE_CHECK, BST_UNCHECKED);

		if( m_pvTorrents->at( nSelection ).m_bZeroSeedInflate )
			CheckDlgButton(IDC_INFLATE_CHECK, BST_CHECKED);
		else
			CheckDlgButton(IDC_INFLATE_CHECK, BST_UNCHECKED);
	}

	*pResult = 0;
}

//
//
//
void CModifyStatsDlg::OnLvnKeydownTorrentList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	switch( pLVKeyDow->wVKey )
	{
	case VK_PRIOR:
		{
			if( m_torrentsList.GetSelectionMark() - 16 < 0 )
			{
				m_torrentsList.SetSelectionMark( 0 );
				OnNMClickTorrentList(NULL, pResult);
			}
			else
			{
				m_torrentsList.SetSelectionMark( m_torrentsList.GetSelectionMark() - 16 );
				OnNMClickTorrentList(NULL, pResult);
			}
			break;
		}
	case VK_NEXT:
		{
			if( m_torrentsList.GetSelectionMark() + 16 > m_torrentsList.GetItemCount() - 1 )
			{
				m_torrentsList.SetSelectionMark( m_torrentsList.GetItemCount() - 1 );
				OnNMClickTorrentList(NULL, pResult);
			}
			else
			{
				m_torrentsList.SetSelectionMark( m_torrentsList.GetSelectionMark() + 16 );
				OnNMClickTorrentList(NULL, pResult);
			}
			break;
		}
	case VK_UP:
		{
			if( m_torrentsList.GetSelectionMark() != 0 )
			{
				m_torrentsList.SetSelectionMark( m_torrentsList.GetSelectionMark() - 1 );
				OnNMClickTorrentList(NULL, pResult);
			}
			break;
		}
	case VK_DOWN:
		{
			m_torrentsList.SetSelectionMark( m_torrentsList.GetSelectionMark() + 1 );
			OnNMClickTorrentList(NULL, pResult);
			break;
		}
	case VK_F5:
		{
			GetTorrentsThreadProc( GetSafeHwnd() );
			m_torrentsList.SetSelectionMark( -1 );
			break;
		}
	}

	*pResult = 0;
}

//
//
//
void CModifyStatsDlg::OnBnClickedEditButton()
{
	if( !m_bEditing && m_torrentsList.GetSelectionMark() != -1 )
	{
		GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Please wait..." );

		GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Attempting to get lock for torrent..." );

		char aSelectQuery[256];
		sprintf( aSelectQuery, "SELECT * FROM bt_generator_maininfo WHERE auto_torrent_id = %u AND ( update_lock = 'F' OR ( unix_timestamp(now()) - unix_timestamp(update_lock_time) > 70 ) )",
			m_pvTorrents->at( m_torrentsList.GetSelectionMark() ).m_nTorrentID );
		char aUpdateQuery[256];
		sprintf( aUpdateQuery, "UPDATE bt_generator_maininfo SET update_lock = 'T',update_lock_time = now() WHERE auto_torrent_id = %u",
			m_pvTorrents->at( m_torrentsList.GetSelectionMark() ).m_nTorrentID );

		TinySQL db;
		if( db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) &&
			db.Query( aSelectQuery, true ) && db.m_nRows == 1 && db.Query( aUpdateQuery, false ) )
		{
			GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Lock granted for 60 seconds" );

			GetDlgItem( IDC_BASE_SEEDS_EDIT )->EnableWindow( true );
			GetDlgItem( IDC_BASE_PEERS_EDIT )->EnableWindow( true );
			GetDlgItem( IDC_BASE_COMPLETES_EDIT )->EnableWindow( true );
			GetDlgItem( IDC_SEED_MULTIPLIER_EDIT )->EnableWindow( true );
			GetDlgItem( IDC_PEERS_MULTIPLIER_EDIT )->EnableWindow( true );
			GetDlgItem( IDC_COMPLETE_MULTIPLIER_EDIT )->EnableWindow( true );

			GetDlgItem( IDC_COMMENT_EDIT )->EnableWindow( true );

			GetDlgItem( IDC_ACTIVE_CHECK )->EnableWindow( true );
			GetDlgItem( IDC_GENERATE_CHECK )->EnableWindow( true );

			GetDlgItem( IDC_EDIT_BUTTON )->EnableWindow( false );
			GetDlgItem( IDC_COMMIT_BUTTON )->EnableWindow( true );
			GetDlgItem( IDC_CANCEL_BUTTON )->EnableWindow( true );

			m_torrentsList.EnableWindow( false );

			m_bEditing = true;
			m_editingStartTime = CTime::GetCurrentTime();
			m_nEditSelection = m_torrentsList.GetSelectionMark();
		}
		else
		{
			GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Lock failed...item cannot be edited" );
		}
	}
}

//
//
//
void CModifyStatsDlg::OnBnClickedCommitButton()
{
	if( m_bEditing )
	{
		GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Please wait..." );

		GetDlgItem( IDC_BASE_SEEDS_EDIT )->EnableWindow( false );
		GetDlgItem( IDC_BASE_PEERS_EDIT )->EnableWindow( false );
		GetDlgItem( IDC_BASE_COMPLETES_EDIT )->EnableWindow( false );
		GetDlgItem( IDC_SEED_MULTIPLIER_EDIT )->EnableWindow( false );
		GetDlgItem( IDC_PEERS_MULTIPLIER_EDIT )->EnableWindow( false );
		GetDlgItem( IDC_COMPLETE_MULTIPLIER_EDIT )->EnableWindow( false );

		GetDlgItem( IDC_COMMENT_EDIT )->EnableWindow( false );

		GetDlgItem( IDC_ACTIVE_CHECK )->EnableWindow( false );
		GetDlgItem( IDC_GENERATE_CHECK )->EnableWindow( false );

		if( ( CTime::GetCurrentTime() - m_editingStartTime ).GetTotalSeconds() <= 60 )
		{
			// query to check if lock is still valid according to the db
			char aSelectQuery[256];
			sprintf( aSelectQuery, "SELECT * FROM bt_generator_maininfo WHERE auto_torrent_id = %u AND update_lock = 'T'",
				m_pvTorrents->at( m_torrentsList.GetSelectionMark() ).m_nTorrentID );

			// get the info for the inflation update
			CString baseSeeds;
			GetDlgItem( IDC_BASE_SEEDS_EDIT )->GetWindowText( baseSeeds );
			CString basePeers;
			GetDlgItem( IDC_BASE_PEERS_EDIT )->GetWindowText( basePeers );
			CString baseCompletes;
			GetDlgItem( IDC_BASE_COMPLETES_EDIT )->GetWindowText( baseCompletes );
			CString seedMultiplier;
			GetDlgItem( IDC_SEED_MULTIPLIER_EDIT )->GetWindowText( seedMultiplier );
			CString peerMultiplier;
			GetDlgItem( IDC_PEERS_MULTIPLIER_EDIT )->GetWindowText( peerMultiplier );
			CString completeMultiplier;
			GetDlgItem( IDC_COMPLETE_MULTIPLIER_EDIT )->GetWindowText( completeMultiplier );

			// generate the update for the inflation table
			char aUpdateInflationQuery[512];
			sprintf( aUpdateInflationQuery, "UPDATE bt_generator_inflation SET base_seeds = %s,seed_inflation_multiplier = %s,base_peers = %s,peer_inflation_multiplier = %s,base_complete = %s,complete_inflation_multiplier = %s WHERE torrent_id = %u",
				baseSeeds, seedMultiplier, basePeers, peerMultiplier, baseCompletes, completeMultiplier,
                m_pvTorrents->at( m_torrentsList.GetSelectionMark() ).m_nTorrentID );

			// get the info for the maininfo update
			CString comment;
			GetDlgItem( IDC_COMMENT_EDIT )->GetWindowText( comment );
			comment.Replace( "\\", "\\\\" );
			comment.Replace( "'", "\\'" );
			bool bActive = ( IsDlgButtonChecked( IDC_ACTIVE_CHECK ) == BST_CHECKED );
			bool bGenerate = ( IsDlgButtonChecked( IDC_GENERATE_CHECK ) == BST_CHECKED );

			// generate the update for the maininfo table
			char aUpdateMaininfoQuery[512];
			sprintf( aUpdateMaininfoQuery, "UPDATE bt_generator_maininfo SET update_lock = 'F',comment='%s',active='%c',generate='%c' WHERE auto_torrent_id = %u",
				comment, ( bActive ? 'T' : 'F' ), ( bGenerate ? 'T' : 'F' ), m_pvTorrents->at( m_torrentsList.GetSelectionMark() ).m_nTorrentID );

			TinySQL db;
			if( db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) &&
				db.Query( aSelectQuery, true ) && db.m_nRows == 1 && db.Query( aUpdateInflationQuery, false ) &&
				db.Query( aUpdateMaininfoQuery, false ) )
			{
				GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Update worked" );
			}
			else
			{
				GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Update failed" );
			}

			GetTorrentsThreadProc( GetSafeHwnd() );
		}
		else
		{
			GetDlgItem( IDC_BASE_SEEDS_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_BASE_PEERS_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_BASE_COMPLETES_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_SEED_MULTIPLIER_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_PEERS_MULTIPLIER_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_COMPLETE_MULTIPLIER_EDIT )->EnableWindow( false );

			GetDlgItem( IDC_COMMENT_EDIT )->EnableWindow( false );

			GetDlgItem( IDC_ACTIVE_CHECK )->EnableWindow( false );
			GetDlgItem( IDC_GENERATE_CHECK )->EnableWindow( false );

			GetDlgItem( IDC_EDIT_BUTTON )->EnableWindow( true );
			GetDlgItem( IDC_COMMIT_BUTTON )->EnableWindow( false );
			GetDlgItem( IDC_CANCEL_BUTTON )->EnableWindow( false );

			m_torrentsList.EnableWindow( true );

			m_bEditing = false;
			m_nEditSelection = -1;

			NMHDR nmhdr;
			LRESULT lresult;
			OnNMClickTorrentList( &nmhdr, &lresult );

			GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Lost lock before commit... try again" );
		}

		GetDlgItem( IDC_EDIT_BUTTON )->EnableWindow( true );
		GetDlgItem( IDC_COMMIT_BUTTON )->EnableWindow( false );
		GetDlgItem( IDC_CANCEL_BUTTON )->EnableWindow( false );

		m_torrentsList.EnableWindow( true );

		m_bEditing = false;
        m_nEditSelection = -1;
	}
}

//
//
//
void CModifyStatsDlg::OnBnClickedCancelButton()
{
	if( m_bEditing )
	{
		GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Please wait..." );

		char aQuery[256];
		sprintf( aQuery, "UPDATE bt_generator_maininfo SET update_lock = 'F' WHERE auto_torrent_id = %u",
			m_pvTorrents->at( m_nEditSelection ).m_nTorrentID );

		TinySQL db;
		if( db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) && db.Query( aQuery, false ) )
		{
			GetDlgItem( IDC_BASE_SEEDS_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_BASE_PEERS_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_BASE_COMPLETES_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_SEED_MULTIPLIER_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_PEERS_MULTIPLIER_EDIT )->EnableWindow( false );
			GetDlgItem( IDC_COMPLETE_MULTIPLIER_EDIT )->EnableWindow( false );

			GetDlgItem( IDC_COMMENT_EDIT )->EnableWindow( false );

			GetDlgItem( IDC_ACTIVE_CHECK )->EnableWindow( false );
			GetDlgItem( IDC_GENERATE_CHECK )->EnableWindow( false );

			GetDlgItem( IDC_EDIT_BUTTON )->EnableWindow( true );
			GetDlgItem( IDC_COMMIT_BUTTON )->EnableWindow( false );
			GetDlgItem( IDC_CANCEL_BUTTON )->EnableWindow( false );

			m_torrentsList.EnableWindow( true );

			m_bEditing = false;
			m_nEditSelection = -1;

			NMHDR nmhdr;
			LRESULT lresult;
			OnNMClickTorrentList( &nmhdr, &lresult );

			GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Lock released" );
		}
		else
		{
			GetDlgItem( IDC_MODIFY_STATS_STATUS_STATIC )->SetWindowText( "Lock release failed... please try again" );
		}
	}
}

//
//
//
void CModifyStatsDlg::OnEnChangeCommentEdit()
{
	if( GetDlgItem( IDC_COMMENT_EDIT )->GetWindowTextLength() > 255 )
	{
		char aComment[256];
		GetDlgItem( IDC_COMMENT_EDIT )->GetWindowText( aComment, 256 );
		GetDlgItem( IDC_COMMENT_EDIT )->SetWindowText( aComment );
	}
}

BEGIN_MESSAGE_MAP(CModifyStatsDlg, CDialog)
	ON_MESSAGE(WM_GET_TORRENTS_RESULT, GetTorrentsResult)
	ON_NOTIFY(NM_CLICK, IDC_TORRENT_LIST, OnNMClickTorrentList)
	ON_BN_CLICKED(IDC_EDIT_BUTTON, OnBnClickedEditButton)
	ON_BN_CLICKED(IDC_COMMIT_BUTTON, OnBnClickedCommitButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, OnBnClickedCancelButton)
	ON_EN_CHANGE(IDC_COMMENT_EDIT, OnEnChangeCommentEdit)
	ON_NOTIFY(LVN_KEYDOWN, IDC_TORRENT_LIST, OnLvnKeydownTorrentList)
END_MESSAGE_MAP()

// CModifyStatsDlg message handlers

//
//
//
UINT GetTorrentsThreadProc(LPVOID pParam)
{
	vector<TorrentEntry> *pvTorrents = new vector<TorrentEntry>;
	unsigned int time = (unsigned int)CTime::GetCurrentTime().GetTime();
	time -= ( 5 * 60 );

	TinySQL db;
	// Try to connect to the database, unlock any stale locked rows, and get the torrent info
	if( db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) &&
		db.Query( CTime( time ).Format( "UPDATE bt_generator_maininfo SET update_lock = 'F' WHERE update_lock_time < '%Y%m%d%H%M%S'" ), false ) &&
		db.Query( "SELECT auto_torrent_id,torrentfilename,info_hash,creation_date,activation_date,comment,total_length,zero_seed_inflate,generate,active,base_seeds,seed_inflation_multiplier,base_peers,peer_inflation_multiplier,base_complete,complete_inflation_multiplier,update_lock FROM bt_generator_maininfo,bt_generator_inflation WHERE bt_generator_maininfo.auto_torrent_id = bt_generator_inflation.torrent_id ORDER BY torrentfilename,auto_torrent_id ASC", true ) )
	{
		TorrentEntry torrent;
		int nYear;
		int nMonth;
		int nDay;
		int nHour;
		int nMinute;
		int nSecond;

		for( unsigned int i = 0; i < db.m_nRows; i++ )
		{
			torrent.m_nTorrentID = (DWORD)atoi( db.m_ppResults[i][0].c_str() );
			torrent.m_filename = db.m_ppResults[i][1];
			torrent.m_hash = db.m_ppResults[i][2];

			sscanf( db.m_ppResults[i][3].c_str(), "%4d%2d%2d%2d%2d%2d", &nYear, &nMonth, &nDay, &nHour, &nMinute, &nSecond );
			sprintf( torrent.m_aCreationDate, "%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond );

			sscanf( db.m_ppResults[i][4].c_str(), "%4d%2d%2d%2d%2d%2d", &nYear, &nMonth, &nDay, &nHour, &nMinute, &nSecond );
			sprintf( torrent.m_aActivationDate, "%04d-%02d-%02d %02d:%02d:%02d", nYear, nMonth, nDay, nHour, nMinute, nSecond );

			torrent.m_comment = db.m_ppResults[i][5];
			torrent.m_nLength = (DWORD)atoi( db.m_ppResults[i][6].c_str() );

			torrent.m_bZeroSeedInflate = ( db.m_ppResults[i][7].compare( "T" ) == 0 );
			torrent.m_bGenerate = ( db.m_ppResults[i][8].compare( "T" ) == 0 );
			torrent.m_bActive = ( db.m_ppResults[i][9].compare( "T" ) == 0 );

			torrent.m_nBaseSeeds = (DWORD)atoi( db.m_ppResults[i][10].c_str() );
			torrent.m_nSeedMultiplier = (DWORD)atoi( db.m_ppResults[i][11].c_str() );
			torrent.m_nBasePeers = (DWORD)atoi( db.m_ppResults[i][12].c_str() );
			torrent.m_nPeerMultiplier = (DWORD)atoi( db.m_ppResults[i][13].c_str() );
			torrent.m_nBaseCompletes = (DWORD)atoi( db.m_ppResults[i][14].c_str() );
			torrent.m_nCompleteMultiplier = (DWORD)atoi( db.m_ppResults[i][15].c_str() );

			torrent.m_bEditable = ( db.m_ppResults[i][16].compare( "F" ) == 0 );

			pvTorrents->push_back( torrent );
		}
	}

	::PostMessage( (HWND)pParam, WM_GET_TORRENTS_RESULT, (WPARAM)pvTorrents, NULL );

	return 0;
}

