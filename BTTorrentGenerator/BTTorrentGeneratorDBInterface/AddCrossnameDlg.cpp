// AddCrossnameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTTorrentGeneratorDBInterface.h"
#include "AddCrossnameDlg.h"
#include "Hash20Byte.h"
#include "..\TinySQL.h"
#include "..\SHA1.h"

// CAddCrossnameDlg dialog

IMPLEMENT_DYNAMIC(CAddCrossnameDlg, CDialog)
CAddCrossnameDlg::CAddCrossnameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddCrossnameDlg::IDD, pParent)
{
}

CAddCrossnameDlg::~CAddCrossnameDlg()
{
}

void CAddCrossnameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_LIST, m_directoryList);
	DDX_Control(pDX, IDC_INFILE_EDIT, m_infileEdit);
	DDX_Control(pDX, IDC_OUTFILE_EDIT, m_outfileEdit);
}


BEGIN_MESSAGE_MAP(CAddCrossnameDlg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_FILE_LIST, OnNMDblclkFileList)
	ON_BN_CLICKED(IDC_GENERATE_BUTTON, OnBnClickedGenerateButton)
	ON_NOTIFY(NM_CLICK, IDC_FILE_LIST, OnNMClickFileList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_FILE_LIST, OnLvnKeydownFileList)
	ON_BN_CLICKED(IDC_CROSSNAME_RADIO, OnBnClickedCrossnameRadio)
	ON_BN_CLICKED(IDC_ADDHASH_RADIO, OnBnClickedAddhashRadio)
END_MESSAGE_MAP()

// CAddCrossnameDlg message handlers

//
//
//
BOOL CAddCrossnameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_iconImagesList.Create( 16, 16, ILC_COLOR24, 2, 2 );
	m_iconImagesList.Add( AfxGetApp()->LoadIcon( IDI_FOLDER ) );
	m_iconImagesList.Add( AfxGetApp()->LoadIcon( IDI_TORRENT ) );
	m_directoryList.SetImageList( &m_iconImagesList, LVSIL_SMALL );

	m_infileEdit.SetWindowText( "" );
	m_outfileEdit.SetWindowText( "" );

	CRect rect;
	m_directoryList.GetWindowRect( &rect );
	int nWidth = rect.Width()-20;

	m_directoryList.InsertColumn( 0, "Filename", LVCFMT_LEFT, nWidth * 7 / 10 );
	m_directoryList.InsertColumn( 1, "Size", LVCFMT_RIGHT, nWidth * 3 / 10 );

	CheckRadioButton( IDC_CROSSNAME_RADIO, IDC_ADDHASH_RADIO, IDC_CROSSNAME_RADIO );

	SetWindowText( GetCurrentPath() );
	PopulateFileList( GetCurrentPath() );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//
// Get the current process running path
//
CString CAddCrossnameDlg::GetCurrentPath()
{
	char path[MAX_PATH+1];
	::GetCurrentDirectory(sizeof(path),path);
	CString cs_path = path;
	return cs_path;
}

//
//
//
void CAddCrossnameDlg::PopulateFileList(CString path)
{
	m_directoryList.DeleteAllItems();
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
				m_directoryList.InsertItem( num_of_folder, file_data.cFileName, 0 );
				m_directoryList.SetItemText( num_of_folder, 1, "Folder" );
				num_of_folder++;
			}
			else if( strstr(strlwr(filename), ".torrent") != NULL )
			{
				int ret = m_directoryList.InsertItem(m_directoryList.GetItemCount(), file_data.cFileName, 1);
				char size[32];
				sprintf(size, "%u KB", ((file_data.nFileSizeHigh * MAXDWORD) + file_data.nFileSizeLow) / 1024 );
				m_directoryList.SetItemText(ret, 1, size);
			}
		}
		found = ::FindNextFile(search_handle, &file_data);
	}
	::FindClose(search_handle);
}

//
//
//
Hash20Byte CAddCrossnameDlg::ComputeInfohash(const char *pFilename)
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

//
//
//
void CAddCrossnameDlg::OnNMClickFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CString torrent = m_directoryList.GetItemText( m_directoryList.GetSelectionMark(), 0 );

	if( torrent.GetLength() > 8 && torrent.Right( 8 ).CompareNoCase( ".torrent" ) == 0 )
		m_infileEdit.SetWindowText( m_directoryList.GetItemText( m_directoryList.GetSelectionMark(), 0 ) );
	else
		m_infileEdit.SetWindowText( "" );

	*pResult = 0;
}

//
//
//
void CAddCrossnameDlg::OnNMDblclkFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	POSITION pos = m_directoryList.GetFirstSelectedItemPosition();
	int index  = m_directoryList.GetNextSelectedItem(pos);
	CString folder = m_directoryList.GetItemText(index,1);
	if(folder == "Folder")
	{
		if(m_directoryList.GetItemText(index,0) != "..")
		{
			CString sub_folder;
			GetWindowText(sub_folder);
			sub_folder += "\\" + m_directoryList.GetItemText(index,0);
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
void CAddCrossnameDlg::OnLvnKeydownFileList(NMHDR *pNMHDR, LRESULT *pResult)
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
void CAddCrossnameDlg::OnBnClickedGenerateButton()
{
	UpdateData(false);

	int nChecked = GetCheckedRadioButton( IDC_CROSSNAME_RADIO, IDC_ADDHASH_RADIO );

	CString path;
	GetWindowText( path );

	CString inTorrent;
	m_infileEdit.GetWindowText( inTorrent );
	if( inTorrent.GetLength() < 9 || inTorrent.Right( 8 ).CompareNoCase( ".torrent" ) != 0 )
	{
		::MessageBox( NULL, "Source torrent is not a valid filename", "Invalid Filename", MB_OK | MB_ICONERROR );
		return;
	}
	CString inTorrentWithPath = path + "\\" + inTorrent;

	CString outTorrent;
	CString outTorrentWithPath;
	if( nChecked == IDC_CROSSNAME_RADIO )
	{
		m_outfileEdit.GetWindowText( outTorrent );
		if( outTorrent.GetLength() < 9 || outTorrent.Right( 8 ).CompareNoCase( ".torrent" ) != 0 )
		{
			::MessageBox( NULL, "Destination torrent is not a valid filename", "Invalid Filename", MB_OK | MB_ICONERROR );
			return;
		}
		outTorrentWithPath = path + "\\" + outTorrent;

		if( inTorrentWithPath.CompareNoCase( outTorrentWithPath ) == 0 )
		{
			::MessageBox( NULL, "Source and destination torrents\nhave the same filename", "Invalid Filename", MB_OK | MB_ICONERROR );
			return;
		}
	}


	Hash20Byte infohash = ComputeInfohash( inTorrentWithPath );
	char aHashBuf[40+1];
	infohash.ToPrintableCharPtr( aHashBuf );

	CFileStatus status;
	CFile::GetStatus( inTorrentWithPath, status );

	char aQuery[2048];
	CString inTorrentEscaped = inTorrent;
	inTorrentEscaped.Replace( "'", "\\'" );

	CString outTorrentEscaped = outTorrent;
	if( nChecked == IDC_CROSSNAME_RADIO )
	{
		outTorrentEscaped.Replace( "'", "\\'" );
		sprintf( aQuery, "INSERT INTO bt_generator_crossnames (original_filename,crossnamed_filename,info_hash,size,creation_date) values ('%s','%s','%s',%u,now())",
			inTorrentEscaped, outTorrentEscaped, aHashBuf, (UINT)status.m_size );
	}
	else
	{
		sprintf( aQuery, "INSERT INTO bt_generator_crossnames (original_filename,crossnamed_filename,info_hash,size,creation_date) values ('Crossname.Generated.Without.Interface','%s','%s',%u,now())",
			inTorrentEscaped, aHashBuf, (UINT)status.m_size );
	}

	bool bError = false;

	TinySQL db;
	if( !db.Init( "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data", 3306 ) ||
        !db.Query( aQuery, false ) )
	{
		bError = true;
		::MessageBox( NULL, "Database entry error! (0x5001)\nbt_generator_crossnames", "Database Error", MB_OK | MB_ICONERROR );
		return;
	}

	if( nChecked == IDC_CROSSNAME_RADIO )
	{
		CFile infile;
		if( infile.Open( inTorrentWithPath, CFile::modeRead | CFile::shareDenyWrite ) )
		{
			char *pBuf = new char[ (size_t)infile.GetLength() ];
			infile.Read( pBuf, (UINT)infile.GetLength() );

			CFile outfile;
			if( outfile.Open( outTorrentWithPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite ) )
			{
				outfile.SetLength( 0 );
				outfile.Write( pBuf, (UINT)infile.GetLength() );
				outfile.Close();
			}
			else
			{
				bError = true;

				::MessageBox( NULL, "Error creating destination torrent!", "Destination Torrent Error", MB_OK | MB_ICONERROR );

				sprintf( aQuery, "DELETE FROM bt_generator_crossnames WHERE original_filename = '%s' AND crossnamed_filename = '%s' AND info_hash = '%s' AND size = %u AND creation_date = now()",
					inTorrentEscaped, outTorrentEscaped, aHashBuf, (UINT)status.m_size );
				db.Query( aQuery, false );
			}

			infile.Close();
			delete pBuf;
		}
		else
		{
			bError = true;

			::MessageBox( NULL, "Error opening source torrent!", "Source Torrent Error", MB_OK | MB_ICONERROR );

			sprintf( aQuery, "DELETE FROM bt_generator_crossnames WHERE original_filename = '%s' AND crossnamed_filename = '%s' AND info_hash = '%s' AND size = %u AND creation_date = now()",
				inTorrentEscaped, outTorrentEscaped, aHashBuf, (UINT)status.m_size );
			db.Query( aQuery, false );
		}

	}

	if( !bError )
	{
		if( nChecked == IDC_CROSSNAME_RADIO )
			::MessageBox( NULL, "Cross-name Creation Successful", "Crossname Status", MB_OK | MB_ICONINFORMATION );
		else
			::MessageBox( NULL, "Hash Addition Successful", "Crossname Status", MB_OK | MB_ICONINFORMATION );
	}

	PopulateFileList( path );
}

//
//
//
void CAddCrossnameDlg::OnBnClickedCrossnameRadio()
{
	m_outfileEdit.EnableWindow( true );
}

//
//
//
void CAddCrossnameDlg::OnBnClickedAddhashRadio()
{
	m_outfileEdit.SetWindowText( "" );
	m_outfileEdit.EnableWindow( false );
}
