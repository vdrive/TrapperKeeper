#include "StdAfx.h"
#include "BTTrackerDll.h"
#include "BTClient.h"
#include "TorrentIndexRequest.h"
#include "TorrentIndexResult.h"
#include "TorrentListUpdate.h"

//
//
//
BTTrackerDll::BTTrackerDll(void)
: m_nConnectedClients(0)
{
}

//
//
//
BTTrackerDll::~BTTrackerDll(void)
{
}

//
//
//
void BTTrackerDll::DllInitialize()
{
	ListeningSocket::Startup();

	m_dlg.Create( IDD_BTTRACKER_DIALOG, CWnd::GetDesktopWindow() );
	m_dlg.Init( this );

	seedRand();

	// Initialize the indexing system
	m_torrentManager.Init( this );
	// Initialize the connection manager
	m_clientManager.Init( this, 0xffffffff );
	// Initialize the listening socket
	m_clientReceiver.Init( this );

	CreateDirectory( "c:\\BTTracker", NULL );
}

//
//
//
void BTTrackerDll::DllUnInitialize()
{
	m_clientReceiver.Stop();
	m_clientManager.KillModules();
	m_torrentManager.Stop();

	ListeningSocket::Cleanup();
}

//
//
//
void BTTrackerDll::DllStart()
{
	// Start the torrent manager
	m_torrentManager.Run();
	// Open the listening socket
	m_clientReceiver.Run();
}

//
//
//
void BTTrackerDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void BTTrackerDll::AddNewClient(SOCKET hSocket)
{
	m_clientManager.AddNewClient( hSocket );
}

//
//
//
void BTTrackerDll::AddTorrent(Torrent *pTorrent)
{
	m_torrentManager.AddTorrent( pTorrent );
}

//
//
//
void BTTrackerDll::GetConnectedPeers(const Hash20Byte *pHash)
{
	m_torrentManager.GetConnectedPeers( pHash );
}

//
//
//
void BTTrackerDll::ConnectedPeers(const set<Peer> *pPeers)
{
	m_dlg.ConnectedPeers( pPeers );
}


//
//
//
void BTTrackerDll::TorrentIndexReq(TorrentIndexRequest *pTorrentIndexRequest)
{
	if( pTorrentIndexRequest == NULL )
		return;

	char aHashBuf[41];
	pTorrentIndexRequest->m_pBTPacket->GetInfoHash().CopyPrintableToCharBuf( aHashBuf );
	if( pTorrentIndexRequest->m_pBTPacket->GetPacketType() == BTPacketParser::TYPE_GET_ANNOUNCE )
	{
		CLogEntry( "Announce for infohash %s", aHashBuf );
	}
	else
	{
		CLogEntry( "Scrape for infohash %s", aHashBuf );
	}

	m_torrentManager.TorrentIndexReq( pTorrentIndexRequest );
}

//
//
//
void BTTrackerDll::TorrentIndexRes(TorrentIndexResult *pTorrentIndexResponse)
{
	m_clientManager.TorrentIndexRes( pTorrentIndexResponse );
}

//
//
//
void BTTrackerDll::CLogEntry(const char *pDataTemplate, ...)
{
	if( pDataTemplate == NULL )
		return;

	static char aCLogBuf[1024];

	va_list ap;
	va_start( ap, pDataTemplate );
	vsprintf( aCLogBuf, pDataTemplate, ap );
	va_end( ap );

	m_dlg.LogEntry( aCLogBuf );
}

//
//
//
void BTTrackerDll::UpdateTorrentList(TorrentListUpdate *pTorrentListUpdate)
{
	m_dlg.UpdateTorrentList( pTorrentListUpdate );
}

//
//
//
void BTTrackerDll::CheckForStaleIPs(void)
{
	m_torrentManager.CheckForStaleIPs();
}

