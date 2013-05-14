// HashIdentifierDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "BTTorrentGeneratorDBInterface.h"
#include "HashIdentifierDlg.h"
#include "CheckTorrentsThreadParam.h"
#include "Hash20Byte.h"
#include "..\TinySQL.h"
#include "..\SHA1.h"
#include <set>
using namespace std;

#define WM_HASHID_INCREMENT_REAL		WM_USER+1
#define WM_HASHID_INCREMENT_DECOY		WM_USER+2
#define WM_HASHID_INCREMENT_INFLATED	WM_USER+3
#define WM_HASHID_INCREMENT_CROSSNAMED	WM_USER+4
#define WM_HASHID_THREAD_DONE			WM_USER+5
#define WM_HASHID_PROGRESS				WM_USER+6
#define WM_HASHID_STATUS				WM_USER+7

UINT CheckTorrentsThreadProc(LPVOID pParam);
Hash20Byte ComputeInfohash(const char *pFilename);

// CHashIdentifierDlg dialog

IMPLEMENT_DYNAMIC(CHashIdentifierDlg, CDialog)

//
//
//
CHashIdentifierDlg::CHashIdentifierDlg(CWnd* pParent /*=NULL*/)
: CDialog(CHashIdentifierDlg::IDD, pParent)
{
}

//
//
//
CHashIdentifierDlg::~CHashIdentifierDlg()
{
}

//
//
//
void CHashIdentifierDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DIRECTORY_LIST, m_torrentFilesList);
	DDX_Control(pDX, IDC_CHECKING_PROGRESS, m_checkingProg);
}

BEGIN_MESSAGE_MAP(CHashIdentifierDlg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_DIRECTORY_LIST, OnNMDblclkFileList)
	ON_BN_CLICKED(IDC_CHECK_TORRENTS_BUTTON, OnBnClickedCheckTorrentsButton)
	ON_MESSAGE(WM_HASHID_INCREMENT_REAL, OnIncrementReal)
	ON_MESSAGE(WM_HASHID_INCREMENT_DECOY, OnIncrementDecoy)
	ON_MESSAGE(WM_HASHID_INCREMENT_INFLATED, OnIncrementInflated)
	ON_MESSAGE(WM_HASHID_INCREMENT_CROSSNAMED, OnIncrementCrossnamed)
	ON_MESSAGE(WM_HASHID_THREAD_DONE, OnCheckTorrentsThreadDone)
	ON_MESSAGE(WM_HASHID_PROGRESS, OnProgress)
	ON_MESSAGE(WM_HASHID_STATUS, OnStatus)
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_KEYDOWN, IDC_DIRECTORY_LIST, OnLvnKeydownDirectoryList)
END_MESSAGE_MAP()

// CHashIdentifierDlg message handlers

//
//
//
BOOL CHashIdentifierDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_iconImagesList.Create( 16, 16, ILC_COLOR24, 2, 2 );
	m_iconImagesList.Add( AfxGetApp()->LoadIcon( IDI_FOLDER ) );
	m_iconImagesList.Add( AfxGetApp()->LoadIcon( IDI_TORRENT ) );

	m_checkingProg.SetRange( 0, 1 );
	m_checkingProg.SetPos( 1 );
	m_torrentFilesList.SetImageList( &m_iconImagesList, LVSIL_SMALL );

	m_nReal = 0;
	m_nDecoy = 0;
	m_nInflated = 0;
	m_nCrossnamed = 0;

	CRect rect;
	m_torrentFilesList.GetWindowRect( &rect );
	int nWidth = rect.Width()-20;

	m_torrentFilesList.InsertColumn( 0, "Filename", LVCFMT_LEFT, nWidth * 7 / 10 );
	m_torrentFilesList.InsertColumn( 1, "Size", LVCFMT_RIGHT, nWidth * 3 / 10 );

	SetWindowText( GetCurrentPath() );
	PopulateFileList( GetCurrentPath() );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//
// Get the current process running path
//
CString CHashIdentifierDlg::GetCurrentPath()
{
	char path[MAX_PATH+1];
	::GetCurrentDirectory(sizeof(path),path);
	CString cs_path = path;
	return cs_path;
}

//
//
//
void CHashIdentifierDlg::PopulateFileList(CString path)
{
	m_torrentFilesList.DeleteAllItems();
	WIN32_FIND_DATA file_data;

	path+="\\*.*";
	HANDLE search_handle = ::FindFirstFile(path, &file_data);

	BOOL found = FALSE;
	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}

	int num_of_folder = 0;
	while(found == TRUE)
	{
		if(strcmp(file_data.cFileName, ".")!=0)
		{
			char filename[256+1];
			strcpy(filename, file_data.cFileName);

			if( (file_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
			{
				m_torrentFilesList.InsertItem( num_of_folder, file_data.cFileName, 0 );
				m_torrentFilesList.SetItemText( num_of_folder, 1, "Folder" );
				num_of_folder++;
			}
			else if( strstr(strlwr(filename), ".torrent") != NULL )
			{
				int ret = m_torrentFilesList.InsertItem(m_torrentFilesList.GetItemCount(), file_data.cFileName, 1);
				char size[32];
				sprintf(size, "%u KB", ((file_data.nFileSizeHigh * MAXDWORD) + file_data.nFileSizeLow) / 1024 );
				m_torrentFilesList.SetItemText(ret, 1, size);
			}
		}
		found = ::FindNextFile(search_handle, &file_data);
	}
	::FindClose(search_handle);
}

//
//
//
void CHashIdentifierDlg::OnNMDblclkFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	POSITION pos = m_torrentFilesList.GetFirstSelectedItemPosition();
	int index  = m_torrentFilesList.GetNextSelectedItem(pos);
	CString folder = m_torrentFilesList.GetItemText(index,1);
	if(folder == "Folder")
	{
		if(m_torrentFilesList.GetItemText(index,0) != "..")
		{
			CString sub_folder;
			GetWindowText(sub_folder);
			sub_folder += "\\" + m_torrentFilesList.GetItemText(index,0);
			SetWindowText(sub_folder);
			PopulateFileList(sub_folder);
		}
		else
		{
			CString current_path;
			GetWindowText(current_path);
			int slash = current_path.ReverseFind('\\');
			current_path.Delete(slash, current_path.GetLength()-slash);
			current_path.FreeExtra();
			SetWindowText(current_path);
			PopulateFileList(current_path);
		}
	}		
	*pResult = 0;
}

//
//
//
void CHashIdentifierDlg::OnBnClickedCheckTorrentsButton()
{
	m_nReal = 0;
	m_nDecoy = 0;
	GetDlgItem( IDC_REAL_NUM_STATIC )->SetWindowText( "0" );
	GetDlgItem( IDC_DECOY_NUM_STATIC )->SetWindowText( "0" );

	m_checkingProg.SetPos( 0 );
	GetDlgItem( IDC_CHECK_TORRENTS_BUTTON )->EnableWindow( false );
	UpdateData( FALSE );

	CheckTorrentsThreadParam *pCheckTorrentsThreadParam = new CheckTorrentsThreadParam();
	pCheckTorrentsThreadParam->m_hwnd = GetSafeHwnd();
	GetWindowText( pCheckTorrentsThreadParam->m_path );

	for( int i=0; i < m_torrentFilesList.GetItemCount(); i++ )
	{
		CString filename = m_torrentFilesList.GetItemText(i, 0);
		CString folder = m_torrentFilesList.GetItemText(i,1);
		if( folder.Find( "Folder" ) == -1 )
			pCheckTorrentsThreadParam->m_vTorrentFiles.push_back( filename );
	}
	if( pCheckTorrentsThreadParam->m_vTorrentFiles.size() == 0 )
	{
		delete pCheckTorrentsThreadParam;
		::MessageBox( NULL, "No torrent files found in current folder.", "No .torrent Files", MB_OK | MB_ICONERROR );
		GetDlgItem( IDC_CHECK_TORRENTS_BUTTON )->EnableWindow( TRUE );
	}
	else
	{
		m_checkingProg.SetRange( 0, (short)pCheckTorrentsThreadParam->m_vTorrentFiles.size() );
		AfxBeginThread( CheckTorrentsThreadProc, (LPVOID)pCheckTorrentsThreadParam, THREAD_PRIORITY_BELOW_NORMAL );
	}
}

//
//
//
void CHashIdentifierDlg::OnLvnKeydownDirectoryList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	switch( pLVKeyDow->wVKey )
	{
		case VK_F5:
		{
			CString path;
			GetWindowText( path );
			PopulateFileList( path );
			break;
		}
	}

	*pResult = 0;
}

//
//
//
LRESULT CHashIdentifierDlg::OnIncrementReal(WPARAM wparam, LPARAM lparam)
{
	m_nReal++;
	char aNumBuf[16];
	sprintf( aNumBuf, "%d", m_nReal );
	GetDlgItem( IDC_REAL_NUM_STATIC )->SetWindowText( aNumBuf );
	UpdateData( false );
	return 0;
}

//
//
//
LRESULT CHashIdentifierDlg::OnIncrementDecoy(WPARAM wparam, LPARAM lparam)
{
	m_nDecoy++;
	char aNumBuf[16];
	sprintf( aNumBuf, "%d", m_nDecoy );
	GetDlgItem( IDC_DECOY_NUM_STATIC )->SetWindowText( aNumBuf );
	UpdateData( false );
	return 0;
}

//
//
//
LRESULT CHashIdentifierDlg::OnIncrementInflated(WPARAM wparam, LPARAM lparam)
{
	m_nInflated++;
	char aNumBuf[16];
	sprintf( aNumBuf, "%d", m_nInflated );
	GetDlgItem( IDC_INFLATION_NUM_STATIC )->SetWindowText( aNumBuf );
	UpdateData( false );
	return 0;
}

//
//
//
LRESULT CHashIdentifierDlg::OnIncrementCrossnamed(WPARAM wparam, LPARAM lparam)
{
	m_nCrossnamed++;
	char aNumBuf[16];
	sprintf( aNumBuf, "%d", m_nCrossnamed );
	GetDlgItem( IDC_CROSSNAME_NUM_STATIC )->SetWindowText( aNumBuf );
	UpdateData( false );
	return 0;
}

//
//
//
LRESULT CHashIdentifierDlg::OnCheckTorrentsThreadDone(WPARAM wparam, LPARAM lparam)
{
	GetDlgItem( IDC_CHECK_TORRENTS_BUTTON )->EnableWindow( true );
	GetDlgItem( IDC_CURRENT_FILE_STATIC )->SetWindowText( "" );

	CString path;
	GetWindowText( path );
	PopulateFileList( path );

	return 0;
}

//
//
//
LRESULT CHashIdentifierDlg::OnProgress(WPARAM wparam, LPARAM lparam)
{
	m_checkingProg.SetPos( (int)wparam );
	return 0;
}

//
//
//
LRESULT CHashIdentifierDlg::OnStatus(WPARAM wparam, LPARAM lparam)
{
	if( wparam != NULL )
	{
		GetDlgItem( IDC_CURRENT_FILE_STATIC )->SetWindowText( (const char*)wparam );
		delete (const char*)wparam;
	}
	return 0;
}

//
//
//
UINT CheckTorrentsThreadProc(LPVOID pParam)
{
	if( pParam == NULL )
		return -1;

	CheckTorrentsThreadParam* pCheckTorrentsThreadParam = (CheckTorrentsThreadParam*)pParam;

	vector<CString> vRealTorrents;

	vector<CString> vDecoyTorrents;
	set<Hash20Byte> sDecoyHashes;

	vector<CString> vInflatedTorrents;
	set<Hash20Byte> sInflatedHashes;

	vector<CString> vCrossnamedTorrents;
	set<Hash20Byte> sCrossnamedHashes;

	char *pStatusMsg = new char[256];
	sprintf( pStatusMsg, "Contacting database for current infohashes..." );
	::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_STATUS, (WPARAM)pStatusMsg, NULL );

	TinySQL db;
	if( db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) )
	{
		if( db.Query( "SELECT info_hash FROM bt_generator_maininfo ORDER BY info_hash ASC", true ) )
		{
			for( unsigned int i = 0; i < db.m_nRows; i++ )
			{
				Hash20Byte hash;
				if( !db.m_ppResults[i][0].empty() )
				{
					hash.FromHexString( db.m_ppResults[i][0].c_str() );
					sDecoyHashes.insert( hash );
				}
			}
		}
		else
		{
			::MessageBox( NULL, "Database query error! (0x1003)\nbt_generator_maininfo", "Database Error", MB_OK | MB_ICONERROR );

			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_PROGRESS, 0, NULL );
			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_THREAD_DONE, NULL, NULL );

			delete pCheckTorrentsThreadParam;
			return -1;
		}

		if( db.Query( "SELECT hash FROM seed_inflation_data ORDER BY hash ASC", true ) )
		{
			for( unsigned int i = 0; i < db.m_nRows; i++ )
			{
				Hash20Byte hash;
				if( !db.m_ppResults[i][0].empty() )
				{
					hash.FromHexString( db.m_ppResults[i][0].c_str() );
					sInflatedHashes.insert( hash );
				}
			}
		}
		else
		{
			::MessageBox( NULL, "Database query error! (0x6001)\nseed_inflation_data", "Database Error", MB_OK | MB_ICONERROR );

			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_PROGRESS, 0, NULL );
			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_THREAD_DONE, NULL, NULL );

			delete pCheckTorrentsThreadParam;
			return -1;
		}

		if( db.Query( "SELECT info_hash FROM bt_generator_crossnames ORDER BY info_hash ASC", true ) )
		{
			for( unsigned int i = 0; i < db.m_nRows; i++ )
			{
				Hash20Byte hash;
				if( !db.m_ppResults[i][0].empty() )
				{
					hash.FromHexString( db.m_ppResults[i][0].c_str() );
					sCrossnamedHashes.insert( hash );
				}
			}
		}
		else
		{
			::MessageBox( NULL, "Database query error! (0x5002)\nbt_generator_crossnames", "Database Error", MB_OK | MB_ICONERROR );

			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_PROGRESS, 0, NULL );
			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_THREAD_DONE, NULL, NULL );

			delete pCheckTorrentsThreadParam;
			return -1;
		}
	}
	db.Reset();

	pStatusMsg = new char[32];
	sprintf( pStatusMsg, "Checking files..." );
	::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_STATUS, (WPARAM)pStatusMsg, NULL );

	Hash20Byte hash;
	for( size_t i = 0; i < pCheckTorrentsThreadParam->m_vTorrentFiles.size(); i++ )
	{
		pStatusMsg = new char[384];
		sprintf( pStatusMsg, "Checking %s", pCheckTorrentsThreadParam->m_vTorrentFiles[i] );
		::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_STATUS, (WPARAM)pStatusMsg, NULL );
		::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_PROGRESS, (WPARAM)i+1, NULL);

		CString filename = pCheckTorrentsThreadParam->m_path;
		filename += "\\";
		filename += pCheckTorrentsThreadParam->m_vTorrentFiles[i];

		hash = ComputeInfohash( filename );

		if( sDecoyHashes.count( hash ) == 1 )
		{
			vDecoyTorrents.push_back( pCheckTorrentsThreadParam->m_vTorrentFiles[i] );
			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_INCREMENT_DECOY, NULL, NULL );
		}
		else if( sCrossnamedHashes.count( hash ) == 1 )
		{
			vCrossnamedTorrents.push_back( pCheckTorrentsThreadParam->m_vTorrentFiles[i] );
			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_INCREMENT_CROSSNAMED, NULL, NULL );
		}
		else if( sInflatedHashes.count( hash ) == 1 )
		{
			vInflatedTorrents.push_back( pCheckTorrentsThreadParam->m_vTorrentFiles[i] );
			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_INCREMENT_INFLATED, NULL, NULL );
		}
		else
		{
			vRealTorrents.push_back(pCheckTorrentsThreadParam->m_vTorrentFiles[i]);
			::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_INCREMENT_REAL, NULL, NULL );
		}
	}

	//moving files
	pStatusMsg = new char[32];
	sprintf( pStatusMsg, "Moving files..." );
	::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_STATUS, (WPARAM)pStatusMsg, NULL );
	::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_PROGRESS, (WPARAM)0, NULL );

	if( vRealTorrents.size() != 0 )
	{
		CString new_folder = pCheckTorrentsThreadParam->m_path;
		new_folder += "\\Real\\";
		CreateDirectory( new_folder, NULL );
		for( UINT i = 0; i < vRealTorrents.size(); i++ )
		{
			CString old_filename = pCheckTorrentsThreadParam->m_path;
			old_filename+="\\";
			old_filename+=vRealTorrents[i];
			
			CString new_filename = new_folder + vRealTorrents[i];
			MoveFile(old_filename, new_filename);
		}
	}
	if( vDecoyTorrents.size() != 0 )
	{
		for( UINT i = 0; i < vDecoyTorrents.size(); i++ )
		{
			CString new_folder = pCheckTorrentsThreadParam->m_path;
			new_folder+="\\Decoys\\";
			CreateDirectory(new_folder,NULL);

			CString old_filename = pCheckTorrentsThreadParam->m_path;
			old_filename+="\\";
			old_filename+=vDecoyTorrents[i];
			
			CString new_filename = new_folder + vDecoyTorrents[i];
			MoveFile(old_filename, new_filename);
		}
	}
	if( vInflatedTorrents.size() != 0 )
	{
		for( UINT i = 0; i < vInflatedTorrents.size(); i++ )
		{
			CString new_folder = pCheckTorrentsThreadParam->m_path;
			new_folder+="\\Inflated\\";
			CreateDirectory(new_folder,NULL);

			CString old_filename = pCheckTorrentsThreadParam->m_path;
			old_filename+="\\";
			old_filename+=vInflatedTorrents[i];
			
			CString new_filename = new_folder + vInflatedTorrents[i];
			MoveFile(old_filename, new_filename);
		}
	}
	if( vCrossnamedTorrents.size() != 0 )
	{
		for( UINT i = 0; i < vCrossnamedTorrents.size(); i++ )
		{
			CString new_folder = pCheckTorrentsThreadParam->m_path;
			new_folder+="\\Crossnames\\";
			CreateDirectory(new_folder,NULL);

			CString old_filename = pCheckTorrentsThreadParam->m_path;
			old_filename+="\\";
			old_filename+=vCrossnamedTorrents[i];
			
			CString new_filename = new_folder + vCrossnamedTorrents[i];
			MoveFile(old_filename, new_filename);
		}
	}

	::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_PROGRESS, (WPARAM)pCheckTorrentsThreadParam->m_vTorrentFiles.size(), NULL );
	::PostMessage( pCheckTorrentsThreadParam->m_hwnd, WM_HASHID_THREAD_DONE, NULL, NULL );
	delete pCheckTorrentsThreadParam;

	return 0;
}

//
//
//
Hash20Byte ComputeInfohash(const char *pFilename)
{
	if( pFilename == NULL )
		return Hash20Byte();

	CFile file;
	if( file.Open( pFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite ) )
	{
		UINT nFileSize = (UINT)file.GetLength();
		file.SeekToBegin();

		if( nFileSize < 128 )
			return Hash20Byte();

		unsigned char *pFileBuf = new unsigned char[ nFileSize ];
		file.Read( pFileBuf, nFileSize );
		file.Close();

		UINT nOffset = 0;
		bool nFound = false;
		for( /* nothing */; nOffset < ( nFileSize - 6 ); nOffset++ )
		{
			if( memcmp( pFileBuf + nOffset, "4:info", 6 ) == 0 )
			{
				nFound = true;
				break;
			}
		}

		if( !nFound )
		{
			delete pFileBuf;
			return Hash20Byte();
		}

		SHA1 sha1;
		sha1.Update( pFileBuf + nOffset + 6, nFileSize - ( nOffset + 6 + 1 ) );
		sha1.Final();

		unsigned char aSHA1Hash[20];
		sha1.GetHash( aSHA1Hash );

		Hash20Byte hash;
		hash = aSHA1Hash;

		delete pFileBuf;
		return hash;
	}
	else
	{
		return Hash20Byte();
	}
}

