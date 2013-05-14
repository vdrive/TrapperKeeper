//
// ConnectionModuleMessageWnd.cpp
//

#include "StdAfx.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModule.h"

BEGIN_MESSAGE_MAP( ConnectionModuleMessageWnd, CWnd )
	ON_MESSAGE( WM_CMOD_INIT_THREAD_DATA, InitThreadData )
	ON_MESSAGE( WM_CMOD_SOCKET_CONNECT, ClientConnect )
	ON_MESSAGE( WM_CMOD_SOCKET_DISCONNECT, ClientDisconnect )
	ON_MESSAGE( WM_CMOD_CLIENT_CONNECT_ERROR, ClientConnectionError )
	ON_MESSAGE( WM_CMOD_CONNECTIONS_REPAIRED, ConnectionsRepaired )
	ON_MESSAGE( WM_CMOD_TORRENT_SEARCH, TorrentSearch )
	ON_MESSAGE( WM_CMOD_LOG_MSG, LogMsg )
	ON_WM_TIMER()
END_MESSAGE_MAP()

//
//
//
ConnectionModuleMessageWnd::ConnectionModuleMessageWnd()
: m_pMod(NULL), m_oneSecondTimer(NULL), m_repairTimer(NULL)
{
}

//
//
//
ConnectionModuleMessageWnd::~ConnectionModuleMessageWnd()
{
}

//
//
//
LRESULT ConnectionModuleMessageWnd::InitThreadData(WPARAM wparam, LPARAM lparam)
{
	m_pMod->InitThreadData( wparam, lparam );

	m_repairTimer = SetTimer( WM_CMOD_AUTO_REPAIR_TIMER, 15 * 60 * 1000, NULL );
	m_oneSecondTimer = SetTimer( WM_CMOD_ONE_SEC_TIMER, 1 * 1000, NULL );

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
LRESULT ConnectionModuleMessageWnd::TorrentSearch(WPARAM wparam, LPARAM lparam)
{
	m_pMod->TorrentSearch( (TorrentSearchRequest *)wparam );

	return 0;
}

//
//
//
void ConnectionModuleMessageWnd::OnTimer(UINT_PTR nIDEvent)
{
	switch( nIDEvent )
	{
	case WM_CMOD_ONE_SEC_TIMER:
		{
			m_pMod->TimerFired();
		}
		break;
	case WM_CMOD_AUTO_REPAIR_TIMER:
		{
			m_pMod->RepairConnections();
		}
		break;
	}

	__super::OnTimer( nIDEvent );
}
