// BTTorrentGeneratorDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "BTTorrentGeneratorDlg.h"
#include "BTInput.h"
 
BEGIN_MESSAGE_MAP(BTTorrentGeneratorDlg, CDialog)
	//
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(BTTorrentGeneratorDlg, CDialog)

//
//
//
BTTorrentGeneratorDlg::BTTorrentGeneratorDlg(CWnd* pParent /*=NULL*/)
: CDialog(BTTorrentGeneratorDlg::IDD, pParent)
{
}

//
//
//
BTTorrentGeneratorDlg::~BTTorrentGeneratorDlg()
{
}

//
//
//
void BTTorrentGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_GENERATED_TORRENTS_LIST, m_torrentList );
	DDX_Control(pDX, IDC_TOTAL_GENERATION_PROGRESS, m_generationProgressBar);
	DDX_Control(pDX, IDC_GENERATION_STATIC, m_generationProgressStatic);
}

//
//
//
BOOL BTTorrentGeneratorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Determine the width of the log window to set the only column to its maximum width
	CRect logRect;
	m_torrentList.GetClientRect( &logRect );

	m_torrentList.SetExtendedStyle( LVS_EX_FULLROWSELECT );
	m_torrentList.InsertColumn( SUB_TORRENT_NAME, "Torrent File", LVCFMT_LEFT, logRect.Width() - 280 );
	m_torrentList.InsertColumn( SUB_INFO_HASH, "Info Hash", LVCFMT_LEFT, 280 );

	m_generationProgressBar.SetRange( 0, 1 );
	m_generationProgressBar.SetPos( 0 );
	m_generationProgressStatic.SetWindowText( "Generation waiting..." );

	return TRUE;
}

//
//
//
void BTTorrentGeneratorDlg::DisplayTorrents(vector<string> *pvFilenames, vector<string> *pvHashes)
{
	if( pvFilenames == NULL || pvHashes == NULL )
		return;

	if( pvFilenames->size() != pvHashes->size() )
	{
		delete pvFilenames;
		delete pvHashes;
		return;
	}

	int nIndex;
	vector<string>::iterator filenameIter;
	vector<string>::iterator hashIter;
	for(filenameIter = pvFilenames->begin(), hashIter = pvHashes->begin();
		filenameIter != pvFilenames->end(); filenameIter++, hashIter++)
	{
		nIndex = m_torrentList.InsertItem( 0, filenameIter->c_str() );
		m_torrentList.SetItemText( nIndex, SUB_INFO_HASH, hashIter->c_str() );
	}

	delete pvFilenames;
	delete pvHashes;
}

//
//
//
void BTTorrentGeneratorDlg::ResetProgressBar(size_t nTotal)
{
	m_generationProgressBar.SetRange( 0, (short)nTotal );
}

//
//
//
void BTTorrentGeneratorDlg::UpdateProgressBar(void)
{
	m_generationProgressBar.SetPos( m_generationProgressBar.GetPos() + 1 );
}

//
//
//
void BTTorrentGeneratorDlg::UpdateProgressText(const char *pMsg)
{
	if( pMsg != NULL )
	{
		m_generationProgressStatic.SetWindowText( pMsg );
		delete pMsg;
	}
}

