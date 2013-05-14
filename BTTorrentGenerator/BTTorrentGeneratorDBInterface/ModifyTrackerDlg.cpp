// ModifyTrackerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTTorrentGeneratorDBInterface.h"
#include "ModifyTrackerDlg.h"
#include "TrackerEntry.h"
#include "..\TinySQL.h"

#define WM_MODIFYTRACKERDLG_GET_TRACKER_RESULT	WM_USER+1
#define WM_MODIFYTRACKERDLG_GET_TRACKER_TIMER	WM_USER+2

UINT GetModifyTrackerTrackersThreadProc(LPVOID pParam);

// CModifyTrackerDlg dialog

IMPLEMENT_DYNAMIC(CModifyTrackerDlg, CDialog)
CModifyTrackerDlg::CModifyTrackerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyTrackerDlg::IDD, pParent)
{
}

CModifyTrackerDlg::~CModifyTrackerDlg()
{
}

//
//
//
BOOL CModifyTrackerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	m_trackersList.GetWindowRect( &rect );
	int nWidth = rect.Width()-20;

	m_trackersList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_trackersList.InsertColumn( 0, "Tracker", LVCFMT_LEFT, nWidth * 8 / 10 );
	m_trackersList.InsertColumn( 1, "Active", LVCFMT_CENTER, nWidth * 1 / 10 );
	m_trackersList.InsertColumn( 2, "IP/URL", LVCFMT_CENTER, nWidth * 1 / 10 );

	CheckRadioButton( IDC_URL_RADIO, IDC_IP_RADIO, IDC_URL_RADIO );

	AfxBeginThread( GetModifyTrackerTrackersThreadProc, (LPVOID)GetSafeHwnd(), THREAD_PRIORITY_BELOW_NORMAL );
	SetTimer( WM_MODIFYTRACKERDLG_GET_TRACKER_TIMER, 1 * 60 * 1000, NULL );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//
//
//
void CModifyTrackerDlg::OnBnClickedAddTrackerButton()
{
	char aTracker[255+1];
	m_trackerEdit.GetWindowText( aTracker, 255+1 );
	int nChecked = GetCheckedRadioButton( IDC_URL_RADIO, IDC_IP_RADIO );

	// IF the tracker string is shorter than the shortest tracker possible OR
	//  does not start with "http://" OR does not end with ":6969/announce"
	if( strlen( aTracker ) < strlen( "http://_:6969/announce" ) || 
		strncmp( aTracker, "http://", 7 ) != 0 ||
		strncmp( aTracker + strlen( aTracker ) - 14, ":6969/announce", 14 ) != 0 )
	{
		::MessageBox( NULL, "Malformed tracker url!", "Tracker Format Error", MB_OK | MB_ICONEXCLAMATION );
		return;
	}

	TinySQL db;
	if( !db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) )
	{
		::MessageBox( NULL, "Could not connect to database!", "Database Error", MB_OK | MB_ICONEXCLAMATION );
		return;
	}

	char aQuery[512];
	sprintf( aQuery, "INSERT into bt_generator_trackers (tracker_url,type,active) values ('%s','%s','T')",
		aTracker, ( nChecked == IDC_URL_RADIO ? "URL" : "IP" ) );
	if( !db.Query( aQuery, true ) )
	{
		::MessageBox( NULL, "Database entry error! (0x7003)\nbt_generator_trackers", "Database Error", MB_OK | MB_ICONERROR );
		return;
	}

	GetModifyTrackerTrackersThreadProc( GetSafeHwnd() );
}

//
//
//
void CModifyTrackerDlg::OnBnClickedSetActiveTrackerButton()
{
	int nSelection = m_trackersList.GetSelectionMark();
	if( nSelection != -1 )
	{
		CString tracker = m_trackersList.GetItemText( nSelection, 0 );
		TinySQL db;
		if( !db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) )
		{
			::MessageBox( NULL, "Could not connect to database!", "Database Error", MB_OK | MB_ICONEXCLAMATION );
			return;
		}

		tracker = "UPDATE bt_generator_trackers SET active='T' WHERE tracker_url = '" + tracker + "'";
		if( !db.Query( tracker, true ) )
		{
			::MessageBox( NULL, "Database update error! (0x7001)\nbt_generator_trackers", "Database Error", MB_OK | MB_ICONERROR );
			return;
		}

		m_trackersList.SetItemText( nSelection, 1, "Active" );
	}
	else
	{
		::MessageBox( NULL, "No tracker selected", "No Tracker Error", MB_OK | MB_ICONERROR );
		return;
	}
}

//
//
//
void CModifyTrackerDlg::OnBnClickedSetInactiveTrackerButton()
{
	int nSelection = m_trackersList.GetSelectionMark();
	if( nSelection != -1 )
	{
		CString tracker = m_trackersList.GetItemText( nSelection, 0 );
		TinySQL db;
		if( !db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) )
		{
			::MessageBox( NULL, "Could not connect to database!", "Database Error", MB_OK | MB_ICONEXCLAMATION );
			return;
		}

		tracker = "UPDATE bt_generator_trackers SET active='F' WHERE tracker_url = '" + tracker + "'";
		if( !db.Query( tracker, true ) )
		{
			::MessageBox( NULL, "Database update error! (0x7002)\nbt_generator_trackers", "Database Error", MB_OK | MB_ICONERROR );
			return;
		}

		m_trackersList.SetItemText( nSelection, 1, "" );
	}
	else
	{
		::MessageBox( NULL, "No tracker selected", "No Tracker Error", MB_OK | MB_ICONERROR );
		return;
	}
}

//
//
//
void CModifyTrackerDlg::OnNMClickTrackersList(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nSelection = m_trackersList.GetSelectionMark();
	if( nSelection != -1 )
	{
		m_trackerEdit.SetWindowText( m_trackersList.GetItemText( nSelection, 0 ) );
	}

	*pResult = 0;
}

//
//
//
void CModifyTrackerDlg::OnLvnKeydownTrackersList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	switch( pLVKeyDow->wVKey )
	{
		case VK_F5:
		{
			GetModifyTrackerTrackersThreadProc( GetSafeHwnd() );
			break;
		}
	}

	*pResult = 0;
}

//
//
//
void CModifyTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TORRENT_LIST, m_trackersList);
	DDX_Control(pDX, IDC_TRACKERS_LIST, m_trackersList);
	DDX_Control(pDX, IDC_TRACKER_EDIT, m_trackerEdit);
}

//
//
//
void CModifyTrackerDlg::OnTimer(UINT nIDEvent)
{
	switch( nIDEvent )
	{
	case WM_MODIFYTRACKERDLG_GET_TRACKER_TIMER:
		{
			AfxBeginThread( GetModifyTrackerTrackersThreadProc, (LPVOID)GetSafeHwnd(), THREAD_PRIORITY_BELOW_NORMAL );
			break;
		}
	}

	__super::OnTimer( nIDEvent );
}

//
//
//
LRESULT CModifyTrackerDlg::GetTrackersResult(WPARAM wparam, LPARAM lparam)
{
	if( wparam == NULL )
		return -1;

	vector<TrackerEntry> *pvTrackers = (vector<TrackerEntry> *)wparam;
	m_trackersList.DeleteAllItems();
	m_trackersList.SetSelectionMark( -1 );
	m_trackerEdit.SetWindowText( "" );
	CheckRadioButton( IDC_URL_RADIO, IDC_IP_RADIO, IDC_URL_RADIO );

	int nInsertPoint;
	for( size_t i = 0; i < pvTrackers->size(); i++ )
	{
		nInsertPoint = m_trackersList.InsertItem( 0, pvTrackers->at( i ).tracker.c_str() );
		m_trackersList.SetItemText( nInsertPoint, 1, ( pvTrackers->at( i ).bActive ? "Active" : "" ) );
		m_trackersList.SetItemText( nInsertPoint, 2, ( pvTrackers->at( i ).eType == TrackerEntry::EntryType::IP ?
			"IP" : "URL" ) );
	}

	delete pvTrackers;

	return 0;
}

BEGIN_MESSAGE_MAP(CModifyTrackerDlg, CDialog)
	ON_MESSAGE(WM_MODIFYTRACKERDLG_GET_TRACKER_RESULT, GetTrackersResult)
	ON_BN_CLICKED(IDC_ADD_TRACKER_BUTTON, OnBnClickedAddTrackerButton)
	ON_BN_CLICKED(IDC_SET_INACTIVE_TRACKER_BUTTON, OnBnClickedSetInactiveTrackerButton)
	ON_NOTIFY(NM_CLICK, IDC_TRACKERS_LIST, OnNMClickTrackersList)
	ON_BN_CLICKED(IDC_SET_ACTIVE_TRACKER_BUTTON, OnBnClickedSetActiveTrackerButton)
	ON_NOTIFY(LVN_KEYDOWN, IDC_TRACKERS_LIST, OnLvnKeydownTrackersList)
END_MESSAGE_MAP()

//
//
//
UINT GetModifyTrackerTrackersThreadProc(LPVOID pParam)
{
	if( pParam == NULL )
		return -1;

	vector<TrackerEntry> *pvTrackers = new vector<TrackerEntry>;

	TinySQL db;
	if( db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) &&
		db.Query( "SELECT tracker_url,active,type FROM bt_generator_trackers ORDER BY tracker_url ASC", true ) )
	{
		for( unsigned int i = 0; i < db.m_nRows; i++ )
		{
			TrackerEntry entry;
			if( !db.m_ppResults[i][0].empty() )
			{
				entry.tracker = db.m_ppResults[i][0];
				entry.bActive = ( db.m_ppResults[i][1].compare( "T" ) == 0 );
				entry.eType = ( db.m_ppResults[i][2].compare( "IP" ) == 0 ? TrackerEntry::EntryType::IP :
					TrackerEntry::EntryType::URL );
				pvTrackers->push_back( entry );
			}
		}
	}

	::PostMessage( (HWND)pParam, WM_MODIFYTRACKERDLG_GET_TRACKER_RESULT, (WPARAM)pvTrackers, NULL );

	return 0;
}
