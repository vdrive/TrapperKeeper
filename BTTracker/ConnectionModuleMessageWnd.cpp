//
// ConnectionModuleMessageWnd.cpp
//

#include "StdAfx.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModule.h"
#include "TorrentIndexRequest.h"

BEGIN_MESSAGE_MAP( ConnectionModuleMessageWnd, CWnd )
	ON_MESSAGE( WM_CMOD_INIT_THREAD_DATA, InitThreadData )
	ON_MESSAGE( WM_CMOD_SOCKET_CONNECT, ClientConnect )
	ON_MESSAGE( WM_CMOD_SOCKET_DISCONNECT, ClientDisconnect )
	ON_MESSAGE( WM_CMOD_CLIENT_CONNECT_ERROR, ClientConnectionError )
	ON_MESSAGE( WM_CMOD_CONNECTIONS_REPAIRED, ConnectionsRepaired )
	ON_MESSAGE( WM_CMOD_TORRENT_INDEX_REQUEST, TorrentIndexReq )
	ON_MESSAGE( WM_CMOD_LOG_MSG, LogMsg )
END_MESSAGE_MAP()

//
//
//
ConnectionModuleMessageWnd::ConnectionModuleMessageWnd()
: m_pMod(NULL)
{

}

//
//
//
LRESULT ConnectionModuleMessageWnd::InitThreadData(WPARAM wparam, LPARAM lparam)
{
	m_pMod->InitThreadData( wparam, lparam );
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::LogMsg(WPARAM wparam, LPARAM lparam)
{
	m_pMod->LogMsg( wparam, lparam );
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::ClientConnect(WPARAM wparam, LPARAM lparam)
{
	m_pMod->ClientConnect( (IPPort *)wparam );
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::ClientDisconnect(WPARAM wparam, LPARAM lparam)
{
	m_pMod->ClientDisconnect((IPPort *)wparam);
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::ClientConnectionError(WPARAM wparam, LPARAM lparam)
{
	// log error
	m_pMod->LogMsg( (WPARAM)dupstring("ERROR: ClientConnectionError" ), NULL );

	// request repair from module
	m_pMod->RepairConnections();

	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::ConnectionsRepaired(WPARAM wparam, LPARAM lparam)
{
	m_pMod->ConnectionsRepaired( (int)wparam );

	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::TorrentIndexReq(WPARAM wparam, LPARAM lparam)
{
	m_pMod->TorrentIndexReq( (TorrentIndexRequest *)wparam );

	return 0;
}
