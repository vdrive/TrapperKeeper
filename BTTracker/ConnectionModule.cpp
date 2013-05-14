//
// ConnectionModule.cpp
//
#include "StdAfx.h"
#include "ConnectionModule.h"
#include "ConnectionManager.h"
#include "ConnectionSockets.h"
#include "TorrentIndexRequest.h"
#include "TorrentIndexResult.h"
#include <afxmt.h>				// for CCriticalSection

//
//
//
UINT ConnectionModuleThreadProc(LPVOID pParam)
{
	// Init message window handle
	if( pParam == NULL )
		return (UINT)(-1);

	seedRand();

	// local non-critical section variables
	HWND hwnd = (HWND)pParam;
	queue<SOCKET> qAcceptedSockets;

	// local critical section (threadData) variables
	CCriticalSection criticalSection;
	queue<SOCKET> *pqAcceptedSockets = NULL;
	queue<TorrentIndexResult *> *pqTorrentSearchResults = NULL;

	// Create the sockets for this module and initialize the data
	ConnectionSockets sockets;
	sockets.m_dlg_hwnd = hwnd;
	int num_reserved_events = sockets.ReturnNumberOfReservedEvents();

	// point all the pointers of the threadDataObj to the local critical variables
	ConnectionModuleThreadData threadData;
	memcpy( &threadData.m_aReservedEvents[0], &sockets.m_events[0], sizeof(WSAEVENT)*4 );
	threadData.m_pqAcceptedSockets = NULL;
	threadData.m_pqTorrentSearchResults = NULL;

	// request initialization of the thread data
	::PostMessage( hwnd, WM_CMOD_INIT_THREAD_DATA, (WPARAM)&criticalSection, (LPARAM)&threadData );

	WSANETWORKEVENTS events;							// what the fired event data was

	DWORD num_events=sockets.ReturnNumberOfEvents();	// 64
	BOOL wait_all=false;
	DWORD timeout=WSA_INFINITE;
	BOOL alertable=false;
	DWORD event;										// which event fired

	while(true)
	{
		event = WSAWaitForMultipleEvents( num_events, &sockets.m_events[0], wait_all, timeout, alertable );

		char buf[16];
		_itoa(event,buf,10);

		if( event == WSA_WAIT_FAILED )
		{
			::MessageBox( NULL, "Wait failed", "ERROR!", MB_OK );
			break;
		}

		// exit event
		if( event == 0 )
		{
			WSAResetEvent( sockets.m_events[event] );	// reset event
			break;
		}

		// accept event
		if( event == 1 )
		{
			// Copy the critical data to the temp variables
			{	// START nested scope for critical section
				CSingleLock singleLock( &criticalSection, true );

				if( threadData.m_pqAcceptedSockets != NULL )
				{
					pqAcceptedSockets = threadData.m_pqAcceptedSockets;
					threadData.m_pqAcceptedSockets = NULL;
				}

				WSAResetEvent(sockets.m_events[event]);		// reset event

			}	// END nested scope for critical section

			if( pqAcceptedSockets != NULL )
			{
				// handle new data that has been moved from the critical data to the local data
				while( pqAcceptedSockets->size() > 0 )
				{
					sockets.AddNewClient( pqAcceptedSockets->front() );
					pqAcceptedSockets->pop();
				}

				delete pqAcceptedSockets;
				pqAcceptedSockets = NULL;
			}
		}

		// search result event
		if( event == 2 )
		{
			{	// START nested scope for critical section
				CSingleLock singleLock( &criticalSection, true );

				if( threadData.m_pqTorrentSearchResults != NULL )
				{
					pqTorrentSearchResults = threadData.m_pqTorrentSearchResults;
					threadData.m_pqTorrentSearchResults = NULL;
				}

				WSAResetEvent(sockets.m_events[event]);		// reset event

			}	// END nested scope for critical section

			if( pqTorrentSearchResults != NULL )
			{
				// handle new data that has been moved from the critical data to the local data
				while( pqTorrentSearchResults->size() > 0 )
				{
					sockets.TorrentIndexRes( pqTorrentSearchResults->front() );
					pqTorrentSearchResults->pop();
				}

				delete pqTorrentSearchResults;
				pqTorrentSearchResults = NULL;
			}
		}

		// repair/check all sockets for current connection status event
		if( event == 3 )
		{
			{	// START nested scope for critical section
				CSingleLock singleLock( &criticalSection, true );
				WSAResetEvent(sockets.m_events[event]);		// reset event
			}

			// start repairing the connection counts
			sockets.RepairConnections();
		}

		// network event
		if( event > 3 )
		{
			WSAResetEvent( sockets.m_events[event] );		// reset event

			int index = event-num_reserved_events;

			events=sockets.m_sockets[index].ReturnNetworkEvents();

			if(events.lNetworkEvents & FD_CONNECT)
			{
				sockets.m_sockets[index].OnConnect(events.iErrorCode[FD_CONNECT_BIT]);
			}
			if(events.lNetworkEvents & FD_READ)
			{
				while( sockets.m_sockets[index].OnReceive(events.iErrorCode[FD_READ_BIT]) );
			}
			if(events.lNetworkEvents & FD_WRITE)
			{
				sockets.m_sockets[index].OnSend(events.iErrorCode[FD_WRITE_BIT]);
			}
			if(events.lNetworkEvents & FD_CLOSE)
			{
				sockets.m_sockets[index].OnClose(events.iErrorCode[FD_CLOSE_BIT]);
			}
			if(events.lNetworkEvents & FD_ACCEPT)
			{
				sockets.m_sockets[index].OnAccept(events.iErrorCode[FD_ACCEPT_BIT]);
			}
		}

	}	// end while(1)

	return 0;	// exit the thread
}



//
//
//
ConnectionModule::ConnectionModule(ConnectionManager *pManager, UINT mod)
: m_pManager(pManager), m_nMod(mod), m_pThread(NULL), m_pThreadData(NULL), m_pCriticalSection(NULL)
, m_bRepairing(false), m_nClientConnections(0)
{
	// Create message window so that it will accept messages posted to it
	if( m_wnd.CreateEx( 0, AfxRegisterWndClass(0), NULL, 0, 0, 0, 0, 0, 0, NULL, NULL ) == false )
		return;

	m_wnd.InitParent(this);

	m_pThread = AfxBeginThread( ConnectionModuleThreadProc, (LPVOID)m_wnd.GetSafeHwnd(), THREAD_PRIORITY_BELOW_NORMAL );
}

//
//
//
ConnectionModule::~ConnectionModule()
{
	// Kill the thread
	if( m_pCriticalSection != NULL )
	{
		WSASetEvent( m_pThreadData->m_aReservedEvents[0] );
		m_pCriticalSection = NULL;
	}

	// Wait for the thread to die
	if( m_pThread != NULL )
	{
		WaitForSingleObject( m_pThread->m_hThread, 5000 );
		m_pThread = NULL;
	}

	// Destroy message window
	m_wnd.DestroyWindow();
}

//
//
//
void ConnectionModule::InitThreadData(WPARAM wparam, LPARAM lparam)
{
	if( wparam == NULL || lparam == NULL )
		return;

	m_pCriticalSection = (CCriticalSection *)wparam;
	m_pThreadData = (ConnectionModuleThreadData *)lparam;

	// lock the rest of this function
	CSingleLock singleLock( m_pCriticalSection, true );

	if( m_qInitialAcceptedConnections.size() > 0 )
	{
		if( m_pThreadData->m_pqAcceptedSockets == NULL )
		{
			m_pThreadData->m_pqAcceptedSockets = new queue<SOCKET>;
		}

		while( m_qInitialAcceptedConnections.size() > 0 )
		{
			m_pThreadData->m_pqAcceptedSockets->push( m_qInitialAcceptedConnections.front() );
			m_qInitialAcceptedConnections.pop();
		}

		WSASetEvent( m_pThreadData->m_aReservedEvents[1] );	// set event
	}
}

//
//
//
void ConnectionModule::LogMsg(WPARAM wparam, LPARAM lparam)
{
	if( wparam != NULL )
		m_pManager->LogMsg( (const char *)wparam );
}

//
//
//
void ConnectionModule::AddNewClient(SOCKET hSocket)
{
	m_nClientConnections++;

	if( m_nClientConnections > (WSA_MAXIMUM_WAIT_EVENTS - CMOD_RESERVED_EVENTS) )
	{
		closesocket(hSocket);
		RepairConnections();
		return;
	}

	if( m_pCriticalSection == NULL )
	{
		m_qInitialAcceptedConnections.push( hSocket );
		return;
	}
	else
	{
		CSingleLock singleLock( m_pCriticalSection, true );

		if( m_pThreadData->m_pqAcceptedSockets == NULL )
		{
			m_pThreadData->m_pqAcceptedSockets = new queue<SOCKET>;
		}

		m_pThreadData->m_pqAcceptedSockets->push( hSocket );
		WSASetEvent( m_pThreadData->m_aReservedEvents[1] );	// set event
	}
}

//
//
//
void ConnectionModule::ClientConnect(IPPort *pIP)
{
	if( pIP != NULL )
	{
		m_pManager->ClientConnect(pIP);
	}
}

//
//
//
void ConnectionModule::ClientDisconnect(IPPort *pIP)
{
	if( pIP != NULL )
	{
		m_nClientConnections--;

		m_pManager->ClientDisconnect(pIP);

		if( m_nClientConnections < 0 )
			RepairConnections();
	}
}

//
//
//
void ConnectionModule::TorrentIndexReq(TorrentIndexRequest *pTorrentIndexRequest)
{
	pTorrentIndexRequest->m_nMod = m_nMod;
	m_pManager->TorrentIndexReq( pTorrentIndexRequest );
}

//
//
//
void ConnectionModule::TorrentIndexRes(TorrentIndexResult *pTorrentIndexResponse)
{
	// IF the result is not null
	if( pTorrentIndexResponse != NULL )
	{
		// IF there is no critical section, what the hell are we doing here?
		if( m_pCriticalSection != NULL )
		{
			if( m_pThreadData->m_pqTorrentSearchResults == NULL )
			{
				m_pThreadData->m_pqTorrentSearchResults = new queue<TorrentIndexResult *>;
			}

			m_pThreadData->m_pqTorrentSearchResults->push( pTorrentIndexResponse );

			WSASetEvent( m_pThreadData->m_aReservedEvents[2] );	// set event
		}
		else
		{
			delete pTorrentIndexResponse;
		}
	}	
}

//
// Called when there a new connection is passed to the thread, but there are no availible connections.
//  The module is then closed to new connections (clinet or verification) and each socket in the thread
//  is checked for connection status.
//
void ConnectionModule::RepairConnections(void)
{
	// IF there is no critical section, what the hell are we doing here?
	if( m_pCriticalSection != NULL )
	{
		// close the module to new clients
		m_bRepairing = true;

		// log that this module is shutting down for repairs
		char *pLogMsg = new char[128];
		sprintf( pLogMsg, "Repairing connection status of module %u", m_nMod );
		m_pManager->LogMsg( pLogMsg );

		// lock the critical section
		CSingleLock singleLock( m_pCriticalSection, true );

		// set the repair event
		WSASetEvent( m_pThreadData->m_aReservedEvents[3] );
	}
}

//
// Called with the current connection counts when all the connections have been checked for current status
//
void ConnectionModule::ConnectionsRepaired(int nCurrentConnections)
{
	m_nClientConnections = nCurrentConnections;

	// open the module
	m_bRepairing = false;

	// log that this module is reopened
	char *pLogMsg = new char[128];
	sprintf( pLogMsg, "Module %d connections repaired: %d connections", m_nMod, nCurrentConnections );
	m_pManager->LogMsg( pLogMsg );
}

