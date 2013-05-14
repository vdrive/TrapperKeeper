// GnutellaFileDownloaderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GnutellaFileDownloader.h"
#include "GnutellaFileDownloaderDlg.h"


// GnutellaFileDownloaderDlg dialog

IMPLEMENT_DYNAMIC(GnutellaFileDownloaderDlg, CDialog)
GnutellaFileDownloaderDlg::GnutellaFileDownloaderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(GnutellaFileDownloaderDlg::IDD, pParent)
{
}

GnutellaFileDownloaderDlg::~GnutellaFileDownloaderDlg()
{
	delete dl;
}

void GnutellaFileDownloaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_log);
}


BEGIN_MESSAGE_MAP(GnutellaFileDownloaderDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

//

BOOL GnutellaFileDownloaderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	WSocket::Startup();

	m_log.InsertColumn( 0, "IP Address", LVCFMT_LEFT, 300 );
	m_log.InsertColumn( 1, "Port", LVCFMT_LEFT, 75 );
	m_log.InsertColumn( 2, "Hash", LVCFMT_LEFT, 75 );
	m_log.InsertColumn( 3, "Filesize", LVCFMT_LEFT, 75 );
	m_log.InsertColumn( 4, "Downloaded", LVCFMT_LEFT, 100 );


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void GnutellaFileDownloaderDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here


	//Connection Info - These are hardcoded currently for testing purposes.
	unsigned int _port=6436;
	CString _ipaddress="216.151.150.196";
	CString _hash="TXMT67FOD65W2MNMIFERCOSFP6L7OZG7";
	unsigned int _filesize=68929;

	Status("Attempting to Connect\0");

	dl = new DownloadingSocket();
	dl->InitParent(this);

	//Set the connection info before we can begin to download.
	dl->SetConnectionInfo(_ipaddress, _port, _hash, _filesize);
	dl->Begin();
}


// GnutellaFileDownloaderDlg message handlers

void GnutellaFileDownloaderDlg::Status(char *status)
{
	if(status != NULL)
	{
		int nInsertPoint = m_log.InsertItem( m_log.GetItemCount(), status );
		m_log.SetItemText( nInsertPoint, 1, "BLAH" );
		m_log.SetItemText( nInsertPoint, 2, "BLAH" );
		
		if( m_log.GetItemCount() > 30 )
			m_log.DeleteItem( 0 );
	}
}

void GnutellaFileDownloaderDlg::Status(CString status)
{
		int nInsertPoint = m_log.InsertItem( m_log.GetItemCount(), status );
		m_log.SetItemText( nInsertPoint, 1, "BLAH" );
		m_log.SetItemText( nInsertPoint, 2, "BLAH" );

		if( m_log.GetItemCount() > 30 )
			m_log.DeleteItem( 0 );
}