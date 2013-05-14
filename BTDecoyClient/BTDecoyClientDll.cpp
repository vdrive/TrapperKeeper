#include "StdAfx.h"
#include "BTDecoyClientDll.h"
#include "TorrentSearchResult.h"
#include "AcceptedSocket.h"
#include "Torrent.h"

//
//
//
BTDecoyClientDll::BTDecoyClientDll(void)
: m_pData(NULL), m_nDataSize(0)
{
}

//
//
//
BTDecoyClientDll::~BTDecoyClientDll(void)
{
}

//
//
//
void BTDecoyClientDll::DllInitialize(void)
{
	seedRand();

	//
	m_dlg.Create( IDD_DECOYCLIENT_DIALOG, CWnd::GetDesktopWindow() );
	m_dlg.InitParent( this );

	m_bCompleteClient = true;
	m_dlg.SetCompletedCheck();

	//
	m_clientReceiver.InitParent( this );
	m_torrentManager.InitParent( this );

	CFile decoyData;
	if( decoyData.Open( "\\syncher\\rcv\\plug-ins\\BTDecoyClient\\TorrentDecoyFile", CFile::modeRead | CFile::shareDenyWrite ) )
	{
		m_nDataSize = (int)decoyData.GetLength();
		m_pData = new unsigned char[ m_nDataSize ];
		decoyData.Read( m_pData, m_nDataSize );
		decoyData.Close();
	}
	else
	{
		m_nDataSize = 0;
		m_pData = NULL;
		LogMsg( "ERROR: Unable to find generator data file" );
	}

	//
	m_connectionManager.InitParent( this );
	m_connectionManager.InitData( m_pData );
	m_connectionManager.InitDataSize( m_nDataSize );
	m_connectionManager.InitClientType( randByte( 1, 10 ) );

	//
	m_databaseManager.InitParent( this );
}

//
//
//
void BTDecoyClientDll::DllUnInitialize()
{
	m_clientReceiver.Stop();
	m_databaseManager.Stop();
	m_torrentManager.Stop();

	if( m_pData != NULL )
	{
		delete m_pData;
		m_pData = NULL;
	}
}

//
//
//
void BTDecoyClientDll::DllStart()
{
	if( m_nDataSize == 0 || m_pData == NULL )
	{
		LogMsg( "Start up failed... incoming connections will not be accepted" );
	}
	else
	{
		m_databaseManager.InitCompleteStatus( m_bCompleteClient );
		m_databaseManager.Run();

		m_torrentManager.Run();

		m_connectionManager.AddModule();
	}
}

//
//
//
void BTDecoyClientDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void BTDecoyClientDll::AddNewClient(const AcceptedSocket *pSocket)
{
	if( pSocket != NULL )
	{
#ifdef _DEBUG
		if( m_dlg.GetMemLeft() < 128 )
#else
		if( m_dlg.GetMemLeft() < 256 )
#endif
		{
			closesocket( pSocket->m_hSocket );
			delete pSocket;
		}
		else
		{
			m_connectionManager.AddNewClient( pSocket );
		}
	}
}

//
//
//
void BTDecoyClientDll::TorrentSearch(TorrentSearchRequest *pTorrentSearchRequest)
{
	m_torrentManager.TorrentSearch( pTorrentSearchRequest );
}

//
//
//
void BTDecoyClientDll::TorrentSearch(TorrentSearchResult *pTorrentSearchResult)
{
	if( pTorrentSearchResult != NULL )
	{
		pTorrentSearchResult->m_bComplete = m_bCompleteClient;
		m_connectionManager.TorrentSearch( pTorrentSearchResult );
	}
}

//
//
//
void BTDecoyClientDll::ReceivedNewTorrents(map<Hash20Byte, const Torrent *> *pmpTorrents)
{
	if( pmpTorrents == NULL )
		return;

	// Update the gui
	m_dlg.UpdateTorrents( pmpTorrents );

	// Update the torrent manager
	m_torrentManager.ReceivedNewTorrents( pmpTorrents );

	// Start up the client receiver
	static bool bGotTorrents = false;
	if( !bGotTorrents )
	{
		m_clientReceiver.Run();
		bGotTorrents = true;
	}
}

//
//
//
void BTDecoyClientDll::LogMsg(const char *pDataTemplate, ...)
{
	if( pDataTemplate == NULL )
		return;

	static char aLogBuf[1024];

	va_list ap;
	va_start( ap, pDataTemplate );
	vsprintf( aLogBuf, pDataTemplate, ap );
	va_end( ap );

	m_dlg.LogMsg( aLogBuf );
}

