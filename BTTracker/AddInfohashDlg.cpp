// AddInfohashDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "BTTrackerDlg.h"
#include "AddInfohashDlg.h"
#include "Torrent.h"


// CAddInfohashDlg dialog

//
//
//
IMPLEMENT_DYNAMIC(CAddInfohashDlg, CDialog)
CAddInfohashDlg::CAddInfohashDlg(CWnd* pParent /*=NULL*/)
: CDialog(CAddInfohashDlg::IDD, pParent), m_pMainDlg(NULL)
{
}

//
//
//
CAddInfohashDlg::~CAddInfohashDlg()
{
}

//
//
//
void CAddInfohashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFOHASH_EDIT, m_infohash);
	DDX_Control(pDX, IDC_LENGTH_EDIT, m_length);
}

//
//
//
BEGIN_MESSAGE_MAP(CAddInfohashDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnClose)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

//
//
//
BOOL CAddInfohashDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_infohash.SetWindowText( "" );
	m_length.SetWindowText( "0" );
	UpdateData(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// CAddInfohashDlg message handlers

//
//
//
void CAddInfohashDlg::OnBnClickedOk()
{
	UpdateData();

	char aHashStr[41];
	ZeroMemory( aHashStr, sizeof(aHashStr) );
	m_infohash.GetWindowText( aHashStr, sizeof(aHashStr) );

	int nLength = 0;
	char aLength[12];
	ZeroMemory( aLength, sizeof(aLength) );
	if( m_length.GetWindowText( aLength, sizeof(aLength) ) > 0 )
	{
		nLength = atoi( aLength );
	}
	else
	{
		::MessageBox( m_hWnd, "Length not valid!", "ERROR", MB_OK | MB_ICONERROR );
		return;
	}

	if( nLength <= 0 )
	{
		::MessageBox( m_hWnd, "Length not valid!", "ERROR", MB_OK | MB_ICONERROR );
		return;
	}

	if( strlen( aHashStr ) == 40 )
	{
		bool bGoodHash = true;

		for( int i = 0; i < 40 && bGoodHash ; i++ )
		{
			if( !isdigit( aHashStr[i] ) && aHashStr[i] != 'a' && aHashStr[i] != 'b' && aHashStr[i] != 'c' && 
				aHashStr[i] != 'd' && aHashStr[i] != 'e' && aHashStr[i] != 'f' )
				bGoodHash = false;
		}

		if( bGoodHash )
		{
			Torrent *pTorrent = new Torrent;
			pTorrent->m_infoHash.FromHexString( aHashStr );
			pTorrent->m_nLength = nLength;
			m_pMainDlg->AddTorrent( pTorrent );

			ShowWindow(SW_HIDE);

			return;
		}
	}
	else
	{
		::MessageBox( m_hWnd, "Hash not valid!", "ERROR", MB_OK | MB_ICONERROR );
	}
}

//
//
//
void CAddInfohashDlg::OnClose()
{
	m_infohash.SetWindowText( "" );
	m_length.SetWindowText( "0" );
	UpdateData(false);

	ShowWindow(SW_HIDE);
}

//
//
//
void CAddInfohashDlg::Init(CBTTrackerDlg *pMainDlg)
{
	m_pMainDlg = pMainDlg;
}