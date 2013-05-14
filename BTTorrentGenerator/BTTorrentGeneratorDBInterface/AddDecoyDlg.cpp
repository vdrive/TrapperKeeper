#include "stdafx.h"
#include "AddDecoyDlg.h"
#include "..\TinySQL.h"
#include <winsock2.h>
#include <vector>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_GET_TRACKERS_RESULT				WM_USER+01
#define WM_SUBMIT_DATA_RESULT				WM_USER+02
#define WM_ADDDECOYDLG_GETTRACKERS_TIMER	WM_USER+03

// CAddDecoyDlg dialog

CAddDecoyDlg::CAddDecoyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddDecoyDlg::IDD, pParent), m_pSubmitDataThread(NULL)
{
}

//
//
//
void CAddDecoyDlg::AutoSizeAdjust(void)
{
	CString filesize;
	unsigned int nTotalFilesize = 0;
	unsigned int nSingleFilesize = 0;

	m_totalFilesizeStatic.GetWindowText( filesize );
	nTotalFilesize = atol( filesize );

	if( nTotalFilesize % 137 == 0 )
	{
		return;
	}
	else
	{
		for( int i = 0; i < m_torrentContentsList.GetItemCount(); i++ )
		{
			nSingleFilesize = atol( m_torrentContentsList.GetItemText( i, 1 ) );
			if( nSingleFilesize / 137 >= 1 )
			{
				nSingleFilesize = nSingleFilesize - ( nTotalFilesize % 137 );;
				nTotalFilesize = nTotalFilesize - ( nTotalFilesize % 137 );

				char aNumBuf[16+1];

				uitoa( nTotalFilesize, aNumBuf );
				m_totalFilesizeStatic.SetWindowText( aNumBuf );

				uitoa( nSingleFilesize, aNumBuf );
				m_torrentContentsList.SetItemText( i, 1, aNumBuf );

				break;
			}
		}
	}

	m_totalFilesizeStatic.GetWindowText( filesize );
	nTotalFilesize = atol( filesize );

	if( nTotalFilesize % 137 != 0 )
	{
		::MessageBox( NULL, "Could not adjust torrent file\nsize to correct decoy size modifier",
			"File Size Error", MB_ICONEXCLAMATION | MB_OK );
		return;
	}

}

//
//
//
UINT GetAddDecoyTrackersThreadProc(LPVOID pParam)
{
	if( pParam == NULL )
		return -1;

	vector<string> *pvTrackers = new vector<string>;

	TinySQL db;
	if( db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) &&
		db.Query( "SELECT tracker_url FROM bt_generator_trackers WHERE active='T' ORDER BY tracker_url ASC", true ) )
	{
		for( unsigned int i = 0; i < db.m_nRows; i++ )
		{
			if( !db.m_ppResults[i][0].empty() )
				pvTrackers->push_back( db.m_ppResults[i][0] );
		}
	}

	::PostMessage( (HWND)pParam, WM_GET_TRACKERS_RESULT, (WPARAM)pvTrackers, NULL );

	return 0;
}

//
//
//
UINT SubmitDataThreadProc(LPVOID pParam)
{
	if( pParam == NULL )
		return -1;

	return 0;
}

//
//
//
void CAddDecoyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TORRENT_FILENAME_EDIT, m_torrentFilenameEdit);
	DDX_Control(pDX, IDC_TORRENT_FILESIZE_NUM_STATIC, m_totalFilesizeStatic);
	DDX_Control(pDX, IDC_TORRENT_FILE_CONTENTS_LIST, m_torrentContentsList);
	DDX_Control(pDX, IDC_ITEM_FILENAME_EDIT, m_itemFilenameEdit);
	DDX_Control(pDX, IDC_ITEM_FILESIZE_EDIT, m_itemFilesizeEdit);
	DDX_Control(pDX, IDC_BASE_DIRECTORY_EDIT, m_baseDirectoryEdit);
	DDX_Control(pDX, IDC_MULTIFILE_TORRENT_CHECK, m_multifileCheck);
	DDX_Control(pDX, IDC_COMMIT_BUTTON, m_commitButton);
	DDX_Control(pDX, IDC_AUTO_SIZE_CHECK, m_autoSizeAdjust);
	DDX_Control(pDX, IDC_TRACKER_COMBO, m_trackerCombo);
	DDX_Control(pDX, IDC_SEED_BASE_EDIT, m_seedsBaseEdit);
	DDX_Control(pDX, IDC_SEED_MULTIPLIER_EDIT, m_seedsMultiplierEdit);
	DDX_Control(pDX, IDC_PEER_BASE_EDIT, m_peersBaseEdit);
	DDX_Control(pDX, IDC_COMPLETE_BASE_EDIT, m_completeBaseEdit);
	DDX_Control(pDX, IDC_PEER_MULTIPLIER_EDIT5, m_peersMultiplierEdit);
	DDX_Control(pDX, IDC_COMPLETE_MULTIPLIER_EDIT, m_completeMultiplierEdit);
	DDX_Control(pDX, IDC_FORCED_TRACKER_EDIT, m_forcedTrackerEdit);
	DDX_Control(pDX, IDC_FORCE_PRIMARY_CHECK, m_forcedTrackerCheck);
	DDX_Control(pDX, IDC_TORRENT_FILESIZE_REMAINING_NUM_STATIC, m_remaininglFilesizeStatic);
	DDX_Control(pDX, IDC_ITEM_FILESIZE_NUM_STATIC, m_commaItemFilesizeStatic);
}

BEGIN_MESSAGE_MAP(CAddDecoyDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ADD_ITEM_BUTTON, OnBnClickedAddItemButton)
	ON_BN_CLICKED(IDC_REMOVE_ITEM_BUTTON, OnBnClickedRemoveItemButton)
	ON_BN_CLICKED(IDC_COMMIT_BUTTON, OnBnClickedCommitButton)
	ON_BN_CLICKED(IDC_CLEAR_BUTTON, OnBnClickedClearButton)
	ON_NOTIFY(NM_CLICK, IDC_TORRENT_FILE_CONTENTS_LIST, OnNMClickTorrentFileContentsList)
	ON_BN_CLICKED(IDC_MULTIFILE_TORRENT_CHECK, OnBnClickedMultifileTorrentCheck)
	ON_MESSAGE(WM_GET_TRACKERS_RESULT, GetTrackersResult)
	ON_MESSAGE(WM_SUBMIT_DATA_RESULT, SubmitDataResult)
	ON_BN_CLICKED(IDC_FORCE_PRIMARY_CHECK, OnBnClickedForcePrimaryCheck)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_ITEM_FILESIZE_EDIT, OnEnChangeItemFilesizeEdit)
	ON_BN_CLICKED(IDC_REFRESH_TRACKERS_BUTTON, OnBnClickedRefreshTrackersButton)
END_MESSAGE_MAP()

//
//
//
BOOL CAddDecoyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//
	m_torrentFilenameEdit.SetWindowText( "example.torrent" );
	m_totalFilesizeStatic.SetWindowText( "0" );
	m_autoSizeAdjust.SetCheck( BST_CHECKED );

	// Determine the width of the log window to set the only column to its maximum width
	CRect logRect;
	m_torrentContentsList.GetClientRect( &logRect );
	m_torrentContentsList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_torrentContentsList.InsertColumn( 0, "Filename", LVCFMT_LEFT, logRect.Width() - 120 );
	m_torrentContentsList.InsertColumn( 1, "Filesize", LVCFMT_LEFT, 120 );

	//
	m_itemFilenameEdit.SetWindowText( "example item.avi" );
	m_itemFilesizeEdit.SetWindowText( "0" );
	m_baseDirectoryEdit.SetWindowText( "" );
	m_multifileCheck.SetCheck( BST_UNCHECKED );

	m_forcedTrackerEdit.SetWindowText( "" );
	m_forcedTrackerEdit.EnableWindow( false );
	m_forcedTrackerCheck.SetCheck( BST_UNCHECKED );

	m_trackerCombo.InsertString( 0, "<Connecting to database for trackers>" );
	m_trackerCombo.SetCurSel( 0 );
	m_trackerCombo.GetWindowRect( &logRect );
	logRect.bottom = logRect.top + 5 * m_trackerCombo.GetItemHeight( -1 ) + logRect.Height();
	m_trackerCombo.SetWindowPos( NULL, 0, 0, logRect.Width(), logRect.Height(), SWP_NOMOVE | SWP_NOZORDER );
	m_trackerCombo.EnableWindow( false );

	m_seedsBaseEdit.SetWindowText( "0" );
	m_seedsMultiplierEdit.SetWindowText( "1" );
	m_peersBaseEdit.SetWindowText( "0" );
	m_peersMultiplierEdit.SetWindowText( "1" );
	m_completeBaseEdit.SetWindowText( "0" );
	m_completeMultiplierEdit.SetWindowText( "1" );

	AfxBeginThread( GetAddDecoyTrackersThreadProc, (LPVOID)GetSafeHwnd(), THREAD_PRIORITY_BELOW_NORMAL );
	SetTimer( WM_ADDDECOYDLG_GETTRACKERS_TIMER, 1 * 60 * 1000, NULL );

	{
		sprintf( m_aLocalIP, "127.0.0.1" );
		char hostName[128+1];
		ZeroMemory( hostName, sizeof(hostName) );
		if( gethostname( hostName, sizeof(hostName) ) != SOCKET_ERROR )
		{
			struct hostent *pHost = gethostbyname( hostName );
			if( pHost != NULL )
			{
				struct sockaddr_in localHost;
				memcpy( &localHost.sin_addr, pHost->h_addr_list[0], pHost->h_length );
				strcpy( m_aLocalIP, inet_ntoa( localHost.sin_addr ) );
			}
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//
//
//
void CAddDecoyDlg::OnBnClickedAddItemButton()
{
	if( m_multifileCheck.GetCheck() == BST_UNCHECKED && m_torrentContentsList.GetItemCount() > 0 )
	{
		::MessageBox( NULL, "Please set torrent to multifile\nbefore adding more than one item",
			"Not Multifile", MB_ICONEXCLAMATION | MB_OK );
		return;
	}
		
	CString baseDirectory;
	m_baseDirectoryEdit.GetWindowText( baseDirectory );
	baseDirectory.TrimLeft().TrimRight();

	if( m_multifileCheck.GetCheck() == BST_CHECKED && baseDirectory.GetLength() == 0 )
	{
		::MessageBox( NULL, "You must include a base directory\nwhen creating a multifile torrent",
			"No Base Directory", MB_ICONEXCLAMATION | MB_OK );
		return;
	}

	CString filename;
	m_itemFilenameEdit.GetWindowText( filename );
	filename.TrimLeft().TrimRight();

	if( m_multifileCheck.GetCheck() == BST_CHECKED &&
		baseDirectory.GetLength() + filename.GetLength() > 192 )
	{
		if( ::MessageBox( NULL, "Warning: Filename and base directory are\nlonger than 192 characters.\n\nContinue to add item?",
			"Long Directory Structure", MB_ICONEXCLAMATION | MB_YESNO ) == IDNO )
		{
			return;
		}
	
	}

	CString filesize;
	m_itemFilesizeEdit.GetWindowText( filesize );
	filesize.TrimLeft().TrimRight();

	{
		unsigned int nItemFilesize = atoi( filesize );
		CString totalsize;
		m_totalFilesizeStatic.GetWindowText( totalsize );
		unsigned int nTotalFilesize = atol( totalsize );

		if( nTotalFilesize > ~nItemFilesize )
		{
			::MessageBox( NULL, "Warning: Item will cause size overflow!\nUse a smaller item size",
				"Item Size", MB_ICONERROR | MB_OK );
			return;
		}
	}

	if( filename.GetLength() > 0 && filesize.GetLength() > 0 )
	{
		LVFINDINFO info;
		info.flags = LVFI_STRING;
		info.psz = filename;
	
		if( m_torrentContentsList.FindItem( &info ) == -1 )
		{
			int nIndex = m_torrentContentsList.InsertItem( 0, filename );

			if( nIndex != -1 )
			{
				m_torrentContentsList.SetItemText( nIndex, 1, filesize );

				unsigned int nTotalFilesize = atol( filesize );
				m_totalFilesizeStatic.GetWindowText( filesize );
				nTotalFilesize += atol( filesize );
				char aNumBuf[16+1];
				uitoa( nTotalFilesize, aNumBuf );

				m_totalFilesizeStatic.SetWindowText( aNumBuf );
				m_itemFilenameEdit.SetWindowText( "" );
				m_itemFilesizeEdit.SetWindowText( "0" );
				m_commaItemFilesizeStatic.SetWindowText( "0" );
			}
		}
	}

	if( m_autoSizeAdjust.GetCheck() == BST_CHECKED )
	{
		AutoSizeAdjust();
	}
}

//
//
//
void CAddDecoyDlg::OnBnClickedRemoveItemButton()
{
	if( m_torrentContentsList.GetSelectionMark() != -1 )
	{
		CString filesize;
		m_totalFilesizeStatic.GetWindowText( filesize );
		unsigned int nTotalFilesize = atol( filesize );

		nTotalFilesize -= atol( m_torrentContentsList.GetItemText( m_torrentContentsList.GetSelectionMark(), 1 ) );

		char aNumBuf[16+1];
		uitoa( nTotalFilesize, aNumBuf );
		m_totalFilesizeStatic.SetWindowText( aNumBuf );

		m_torrentContentsList.DeleteItem( m_torrentContentsList.GetSelectionMark() );

		if( m_autoSizeAdjust.GetCheck() == BST_CHECKED )
		{
			AutoSizeAdjust();
		}

		m_torrentContentsList.SetSelectionMark( -1 );
	}
}

//
//
//
void CAddDecoyDlg::OnBnClickedCommitButton()
{
	if( m_torrentContentsList.GetItemCount() == 0 )
		return;

	CString baseDirectory;
	m_baseDirectoryEdit.GetWindowText( baseDirectory );
	baseDirectory.TrimLeft().TrimRight();

	if( m_multifileCheck.GetCheck() == BST_CHECKED && baseDirectory.GetLength() == 0 )
	{
		::MessageBox( NULL, "You must include a base directory\nwhen creating a multifile torrent", "No Base Directory", MB_ICONEXCLAMATION | MB_OK );
		return;
	}

	CString primaryTracker;
	CString secondaryTracker;

	if( m_forcedTrackerCheck.GetCheck() == BST_CHECKED )
	{
		m_forcedTrackerEdit.GetWindowText( primaryTracker );
		primaryTracker.TrimLeft().TrimRight();
		if( primaryTracker.GetLength() == 0 || primaryTracker.Left( 7 ).Compare( "http://" ) != 0 )
		{
			::MessageBox( NULL, "Forced Tracker selection error", "Tracker Error", MB_ICONEXCLAMATION | MB_OK );
			return;
		}

		m_trackerCombo.GetLBText( m_trackerCombo.GetCurSel(), secondaryTracker );
		secondaryTracker.TrimLeft().TrimRight();
		if( secondaryTracker.GetLength() == 0 || secondaryTracker.Left( 7 ).Compare( "http://" ) != 0 )
		{
			::MessageBox( NULL, "Tracker selection error", "Tracker Error", MB_ICONEXCLAMATION | MB_OK );
			return;
		}
	}
	else
	{
		m_trackerCombo.GetLBText( m_trackerCombo.GetCurSel(), primaryTracker );
		primaryTracker.TrimLeft().TrimRight();
		if( primaryTracker.GetLength() == 0 || primaryTracker.Left( 7 ).Compare( "http://" ) != 0 )
		{
			::MessageBox( NULL, "Tracker selection error", "Tracker Error", MB_ICONEXCLAMATION | MB_OK );
			return;
		}
	}

	if( ::MessageBox( NULL, "Are you sure?", "Commiting Data", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2 ) == IDNO )
		return;

	m_commitButton.EnableWindow( false );

	TinySQL db;
	if( !db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) )
	{
		::MessageBox( NULL, "Could not connect to database!", "Database Error", MB_OK | MB_ICONEXCLAMATION );
		m_commitButton.EnableWindow( true );
		return;
	}

	CString torrentFilename;
	m_torrentFilenameEdit.GetWindowText( torrentFilename );
	torrentFilename.TrimLeft().TrimRight();

	CString totalLength;
	m_totalFilesizeStatic.GetWindowText( totalLength );
	totalLength.TrimLeft().TrimRight();

	char aQuery[2048+1];
	char aItemFilename[256+1];
	if( m_multifileCheck.GetCheck() == BST_UNCHECKED )
	{
		m_torrentContentsList.GetItemText( 0, 0, aItemFilename, 256 );
		sprintf( aQuery, "INSERT INTO bt_generator_maininfo (torrentfilename,topmost_file_or_dir,created_by,total_length,generate,active) values ('%s','%s','%s','%s','F','T')",
			torrentFilename, aItemFilename, m_aLocalIP, totalLength );
	}
	else
	{
		sprintf( aQuery, "INSERT INTO bt_generator_maininfo (torrentfilename,topmost_file_or_dir,created_by,total_length,generate,active) values ('%s','%s','%s','%s','F','T')",
			torrentFilename, baseDirectory, m_aLocalIP, totalLength );
	}

	if( !db.Query( aQuery, false ) )
	{
		::MessageBox( NULL, "Database entry error! (0x1001)\nbt_generator_maininfo", "Database Error", MB_OK | MB_ICONEXCLAMATION );
		m_commitButton.EnableWindow( true );
		return;
	}

	if( m_multifileCheck.GetCheck() == BST_UNCHECKED )
	{
		sprintf( aQuery, "SELECT auto_torrent_id FROM bt_generator_maininfo WHERE torrentfilename = '%s' AND topmost_file_or_dir = '%s' AND total_length = '%s' ORDER BY auto_torrent_id DESC LIMIT 1",
			torrentFilename, aItemFilename, totalLength );
	}
	else
	{
		sprintf( aQuery, "SELECT auto_torrent_id FROM bt_generator_maininfo WHERE torrentfilename = '%s' AND topmost_file_or_dir = '%s' AND total_length = '%s' ORDER BY auto_torrent_id DESC LIMIT 1",
			torrentFilename, baseDirectory, totalLength );
	}

	if( !db.Query( aQuery, true ) || db.m_nRows == 0 )
	{
		::MessageBox( NULL, "Database entry error! (0x1002)\nbt_generator_maininfo", "Database Error", MB_OK | MB_ICONEXCLAMATION );
		m_commitButton.EnableWindow( true );
		return;
	}

	unsigned int nTorrentID = atol( db.m_ppResults[0][0].c_str() );

	char aItemLength[16+1];
	for( int i = 0; i < m_torrentContentsList.GetItemCount(); i++ )
	{
		m_torrentContentsList.GetItemText( i, 0, aItemFilename, 256 );
		m_torrentContentsList.GetItemText( i, 1, aItemLength, 16 );

		sprintf( aQuery, "INSERT INTO bt_generator_datafile (file_number,file_length,file_name,path,torrent_id) values (%d,%s,'%s','%s',%u)",
			i+1, aItemLength, aItemFilename, baseDirectory, nTorrentID );

		if( !db.Query( aQuery, true ) )
		{
			::MessageBox( NULL, "Database entry error! (0x2001)\nbt_generator_datafile", "Database Error", MB_OK | MB_ICONEXCLAMATION );
			m_commitButton.EnableWindow( true );
			return;
		}
	}

	sprintf( aQuery, "INSERT INTO bt_generator_announcelist (torrent_id,primary_tracker_url,secondary_tracker_url,tertiary_tracker_url) VALUES (%u,'%s','%s','')",
		nTorrentID, primaryTracker, secondaryTracker );
	if( !db.Query( aQuery, true ) )
	{
		::MessageBox( NULL, "Database entry error! (0x3001)\nbt_generator_announcelist", "Database Error", MB_OK | MB_ICONEXCLAMATION );
		m_commitButton.EnableWindow( true );
		return;
	}

	CString seedsBase;
	m_seedsBaseEdit.GetWindowText( seedsBase );
	CString seedsMultiplier;
	m_seedsMultiplierEdit.GetWindowText( seedsMultiplier );
	CString peersBase;
	m_peersBaseEdit.GetWindowText( peersBase );
	CString peersMultiplier;
	m_peersMultiplierEdit.GetWindowText( peersMultiplier );
	CString completeBase;
	m_completeBaseEdit.GetWindowText( completeBase );
	CString completeMultiplier;
	m_completeMultiplierEdit.GetWindowText( completeMultiplier );

	sprintf( aQuery, "INSERT INTO bt_generator_inflation (torrent_id,base_seeds,seed_inflation_multiplier,base_peers,peer_inflation_multiplier,base_complete,complete_inflation_multiplier) VALUES (%u,%s,%s,%s,%s,%s,%s)",
		nTorrentID, seedsBase, seedsMultiplier, peersBase, peersMultiplier, completeBase, completeMultiplier );
	if( !db.Query( aQuery, true ) )
	{
		::MessageBox( NULL, "Database entry error! (0x4001)\nbt_generator_inflation", "Database Error", MB_OK | MB_ICONEXCLAMATION );
		m_commitButton.EnableWindow( true );
		return;
	}
		
	sprintf( aQuery, "UPDATE bt_generator_maininfo SET zero_seed_inflate='%c' WHERE auto_torrent_id = %u",
		( m_forcedTrackerCheck.GetCheck() == BST_CHECKED ? 'T' : 'F' ), nTorrentID );
	if( !db.Query( aQuery, false ) )
	{
		::MessageBox( NULL, "Database entry error! (0x1003)\nbt_generator_maininfo", "Database Error", MB_OK | MB_ICONEXCLAMATION );
		m_commitButton.EnableWindow( true );
		return;
	}

	sprintf( aQuery, "UPDATE bt_generator_maininfo SET generate='T' WHERE auto_torrent_id = %u", nTorrentID );
	if( !db.Query( aQuery, false ) )
	{
		::MessageBox( NULL, "Database entry error! (0x1004)\nbt_generator_maininfo", "Database Error", MB_OK | MB_ICONEXCLAMATION );
		m_commitButton.EnableWindow( true );
		return;
	}

	::MessageBox( NULL, "Data entered", "Database Success", MB_OK );

	m_commitButton.EnableWindow( true );

	OnBnClickedClearButton();
}

//
//
//
void CAddDecoyDlg::OnBnClickedClearButton()
{
	m_torrentFilenameEdit.SetWindowText( "" );
	m_totalFilesizeStatic.SetWindowText( "0" );
	m_torrentContentsList.DeleteAllItems();
	m_itemFilenameEdit.SetWindowText( "" );
	m_itemFilesizeEdit.SetWindowText( "0" );
	m_commaItemFilesizeStatic.SetWindowText( "0" );
	m_baseDirectoryEdit.SetWindowText( "" );
	m_multifileCheck.SetCheck( BST_UNCHECKED );
	m_baseDirectoryEdit.EnableWindow( false );

	m_trackerCombo.SetCurSel( 0 );

	m_forcedTrackerEdit.SetWindowText( "" );
	m_forcedTrackerEdit.EnableWindow( false );
	m_forcedTrackerCheck.SetCheck( BST_UNCHECKED );

	m_seedsBaseEdit.SetWindowText( "0" );
	m_seedsMultiplierEdit.SetWindowText( "1" );
	m_peersBaseEdit.SetWindowText( "0" );
	m_peersMultiplierEdit.SetWindowText( "1" );
	m_completeBaseEdit.SetWindowText( "0" );
	m_completeMultiplierEdit.SetWindowText( "1" );
}

//
//
//
void CAddDecoyDlg::OnNMClickTorrentFileContentsList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if( m_torrentContentsList.GetSelectionMark() == -1 )
	{
		m_itemFilenameEdit.SetWindowText( "" );
		m_itemFilesizeEdit.SetWindowText( "0" );
		m_commaItemFilesizeStatic.SetWindowText( "0" );
	}
	else
	{
		char aBuf[256+1];
		aBuf[0] = '\0';

		m_torrentContentsList.GetItemText( m_torrentContentsList.GetSelectionMark(), 0, aBuf, 256 );
		m_itemFilenameEdit.SetWindowText( aBuf );

		m_torrentContentsList.GetItemText( m_torrentContentsList.GetSelectionMark(), 1, aBuf, 256 );
		m_itemFilesizeEdit.SetWindowText( aBuf );
	}

	*pResult = 0;
}

//
//
//
void CAddDecoyDlg::OnBnClickedMultifileTorrentCheck()
{
	if( m_multifileCheck.GetCheck() == BST_UNCHECKED )
	{
		m_baseDirectoryEdit.SetWindowText( "" );
		m_baseDirectoryEdit.EnableWindow( false );
	}
	else
	{
		m_baseDirectoryEdit.SetWindowText( "" );
		m_baseDirectoryEdit.EnableWindow( true );
	}

	m_torrentContentsList.DeleteAllItems();
	m_totalFilesizeStatic.SetWindowText( "0" );
}

//
//
//
LRESULT CAddDecoyDlg::GetTrackersResult(WPARAM wparam, LPARAM lparam)
{
	// IF no trackers were returned
	if( ( wparam == NULL || ((vector<string> *)wparam)->size() == 0 ) && ( m_trackerCombo.GetCount() == 1 ) )
	{
		CString tracker;
		m_trackerCombo.GetLBText( 0, tracker );
		if( tracker.Compare( "<Tracker DB Error - Please restart BT Interface tool>" ) == 0 ||
			tracker.Compare( "<Connecting to database for trackers>" ) == 0 ||
			tracker.Compare( "<None>" ) == 0 )
		{
			m_trackerCombo.DeleteString( 0 );
			m_trackerCombo.InsertString( 0, "<Tracker DB Error - Please restart BT Interface tool>" );
			m_trackerCombo.SetCurSel( 0 );
			m_forcedTrackerCheck.EnableWindow( false );
		}
		return 1;
	}

	while( m_trackerCombo.GetCount() > 0 )
		m_trackerCombo.DeleteString( 0 );

	vector<string> *pvTrackers = (vector<string> *)wparam;

	for( size_t i = 0; i < pvTrackers->size(); i++ )
		m_trackerCombo.InsertString( (int)i, pvTrackers->at( i ).c_str() );

	delete pvTrackers;

	m_trackerCombo.EnableWindow( true );
	m_trackerCombo.SetCurSel( 0 );

	return 0;
}

//
//
//
LRESULT CAddDecoyDlg::SubmitDataResult(WPARAM wparam, LPARAM lparam)
{
	return 0;
}

//
//
//
void CAddDecoyDlg::OnBnClickedForcePrimaryCheck()
{
	if( m_forcedTrackerCheck.GetCheck() == BST_UNCHECKED )
	{
		m_forcedTrackerEdit.SetWindowText( "" );
		m_forcedTrackerEdit.EnableWindow( false );
	}
	else
	{
		m_forcedTrackerEdit.EnableWindow( true );
	}
}

//
//
//
void CAddDecoyDlg::OnBnClickedRefreshTrackersButton()
{
	GetAddDecoyTrackersThreadProc( GetSafeHwnd() );
}

//
//
//
void CAddDecoyDlg::OnEnChangeItemFilesizeEdit()
{
	if( m_itemFilesizeEdit.GetWindowTextLength() > 10 )
	{
		m_commaItemFilesizeStatic.SetWindowText( "4,294,967,295" );
		m_itemFilesizeEdit.SetWindowText( "4294967295" );

		::MessageBox( NULL, "Value too large for torrent\nSetting to max possible value", "Value Error", MB_OK | MB_ICONERROR );
		return;
	}
	else if( m_itemFilesizeEdit.GetWindowTextLength() == 10 )
	{
		char aBuf[10+1];
		m_itemFilesizeEdit.GetWindowText( aBuf, 10+1 );
		unsigned int nFilesize = (unsigned int)atoi( aBuf );

		if( ( aBuf[0] == '4' && aBuf[1] == '3' ) || ( aBuf[0] == '4' && aBuf[1] == '4' ) ||
			( aBuf[0] == '4' && aBuf[1] == '5' ) || ( aBuf[0] == '4' && aBuf[1] == '6' ) ||
			( aBuf[0] == '4' && aBuf[1] == '7' ) || ( aBuf[0] == '4' && aBuf[1] == '8' ) ||
			( aBuf[0] == '4' && aBuf[1] == '9' ) || aBuf[0] == '5' || aBuf[0] == '6' ||
			aBuf[0] == '7' || aBuf[0] == '8' || aBuf[0] == '9' || nFilesize <= 5032705 )
		{
			m_commaItemFilesizeStatic.SetWindowText( "4,294,967,295" );
			m_itemFilesizeEdit.SetWindowText( "4294967295" );

			::MessageBox( NULL, "Value too large for torrent\nSetting to max possible value", "Value Error", MB_OK | MB_ICONERROR );
			return;
		}
	}

	CString buf;
	m_itemFilesizeEdit.GetWindowText( buf );
	int nLen = buf.GetLength();

	switch( nLen )
	{
	case 10:
		{
			buf.Insert( buf.GetLength() - 9, ',' );
			buf.Insert( buf.GetLength() - 6, ',' );
			buf.Insert( buf.GetLength() - 3, ',' );
		}
		break;
	case 9:
	case 8:
	case 7:
		{
			buf.Insert( buf.GetLength() - 6, ',' );
			buf.Insert( buf.GetLength() - 3, ',' );
		}
		break;
	case 6:
	case 5:
	case 4:
		{
			buf.Insert( buf.GetLength() - 3, ',' );
		}
		break;
	}

	buf.Insert( 0, '(' );
	buf.Insert( buf.GetLength(), ')' );
	m_commaItemFilesizeStatic.SetWindowText( buf );
}

//
//
//
void CAddDecoyDlg::OnTimer(UINT nIDEvent)
{
	switch( nIDEvent )
	{
	case WM_ADDDECOYDLG_GETTRACKERS_TIMER:
		{
			AfxBeginThread( GetAddDecoyTrackersThreadProc, (LPVOID)GetSafeHwnd(), THREAD_PRIORITY_BELOW_NORMAL );
			break;
		}
	}

	__super::OnTimer( nIDEvent );
}
