//
// ConnectionModule.cpp
//
#include "StdAfx.h"
#include "ConnectionModule.h"
#include "ConnectionModuleThreadParam.h"
#include "ConnectionModuleThreadData.h"
#include "ConnectionManager.h"
#include "ConnectionSockets.h"
#include "TorrentSearchRequest.h"
#include "TorrentSearchResult.h"
#include "Torrent.h"

#include <afxmt.h>				// for CCriticalSection

//
//
//
unsigned int ConnectionModuleThreadProc(LPVOID pParam)
{
	// Init message window handle
	if( pParam == NULL )
		return (unsigned int)(-1);

	HWND hwnd = ((ConnectionModuleThreadParam *)pParam)->m_hwnd;
	queue<SOCKET> qAcceptedSockets;

	// local critical section (threadData) variables
	CCriticalSection criticalSection;
	queue<SOCKET> *pqAcceptedSockets = NULL;

	// Create the sockets for this module and initialize the data
	seedRand();
	ConnectionSockets sockets( ((ConnectionModuleThreadParam *)pParam)->m_hwnd,
        ((ConnectionModuleThreadParam *)pParam)->m_pData, ((ConnectionModuleThreadParam *)pParam)->m_nDataSize,
        ((ConnectionModuleThreadParam *)pParam)->m_nClientType );
	delete (ConnectionModuleThreadParam *)pParam;

	int num_reserved_events = sockets.ReturnNumberOfReservedEvents();

	// point all the pointers of the threadDataObj to the local critical variables
	ConnectionModuleThreadData threadData( sockets.m_events );
	threadData.m_pqAcceptedSockets = NULL;
	threadData.m_pqSearchResults = NULL;

	// request initialization of the thread data
	::PostMessage( hwnd, WM_CMOD_INIT_THREAD_DATA, (WPARAM)&criticalSection, (LPARAM)&threadData );

	WSANETWORKEVENTS events;									// what the fired event data was

	unsigned long num_events=sockets.ReturnNumberOfEvents();	// 64
	BOOL wait_all=false;
	unsigned long timeout=WSA_INFINITE;
	BOOL alertable=false;
	unsigned long event;										// which event fired

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
			queue<TorrentSearchResult *> *pqTorrentSearchResults;

			// Copy the critical data to the temp variables
			{	// START nested scope for critical section
				CSingleLock singleLock( &criticalSection, true );

				if( threadData.m_pqSearchResults != NULL )
				{
					pqTorrentSearchResults = threadData.m_pqSearchResults;
					threadData.m_pqSearchResults = NULL;
				}

				WSAResetEvent(sockets.m_events[event]);		// reset event

			}	// END nested scope for critical section

			if( pqTorrentSearchResults != NULL )
			{
				// handle new data that has been moved from the critical data to the local data
				while( pqTorrentSearchResults->size() > 0 )
				{
					sockets.TorrentSearch( pqTorrentSearchResults->front() );
					pqTorrentSearchResults->pop();
				}

				delete pqTorrentSearchResults;
				pqTorrentSearchResults = NULL;
			}
		}

		// repair/check all sockets for current connection status event OR
		//  2 minute ping for keep alive
		if( event == 3 )
		{
			bool bRepair = false;
			bool bTimer = false;

			{	// START nested scope for critical section
				CSingleLock singleLock( &criticalSection, true );
				
				bRepair = threadData.m_bRepair;
				threadData.m_bRepair = false;

				bTimer = threadData.m_bTimer;
				threadData.m_bTimer = false;

				WSAResetEvent(sockets.m_events[event]);		// reset event
			}

			if( bRepair )
			{
				sockets.RepairConnections();
			}

			if( bTimer )
			{
				sockets.TimerFired();
			}
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

	if( pqAcceptedSockets != NULL )
	{
		delete pqAcceptedSockets;
		pqAcceptedSockets = NULL;
	}

	return 0;	// exit the thread
}



//
//
//
ConnectionModule::ConnectionModule(ConnectionManager *pManager, unsigned int mod,
								   const unsigned char *pData, int nDataSize, int nClientType)
: m_pManager(pManager), m_nMod(mod), m_pThread(NULL), m_pThreadData(NULL)
, m_pCriticalSection(NULL), m_bRepairing(false), m_nClientConnections(0)
{
	// Create message window so that it will accept messages posted to it
	if( m_wnd.CreateEx( 0, AfxRegisterWndClass(0), NULL, 0, 0, 0, 0, 0, 0, NULL, NULL ) == false )
		return;

	m_wnd.InitParent(this);

	m_pThread = AfxBeginThread( ConnectionModuleThreadProc,
		(LPVOID)new ConnectionModuleThreadParam( m_wnd.GetSafeHwnd(), pData, nDataSize, nClientType ), THREAD_PRIORITY_BELOW_NORMAL );
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
		WaitForSingleObject( m_pThread->m_hThread, 3000 );
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
		m_nClientConnections = (WSA_MAXIMUM_WAIT_EVENTS - CMOD_RESERVED_EVENTS);
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
		{
			m_nClientConnections = 0;
			RepairConnections();
		}
	}
}

//
//
//
void ConnectionModule::TorrentSearch(TorrentSearchRequest *pTorrentSearchRequest)
{
	if( pTorrentSearchRequest != NULL )
	{
		pTorrentSearchRequest->SetConnectionModule( m_nMod );
		m_pManager->TorrentSearch( pTorrentSearchRequest );
	}
}

//
//
//
void ConnectionModule::TorrentSearch(TorrentSearchResult *pTorrentSearchResult)
{
	if( pTorrentSearchResult != NULL )
	{
		if( m_pCriticalSection == NULL )
		{
			delete pTorrentSearchResult;
		}
		else
		{
			CSingleLock singleLock( m_pCriticalSection, true );

			if( m_pThreadData->m_pqSearchResults == NULL )
			{
				m_pThreadData->m_pqSearchResults = new queue<TorrentSearchResult *>;
			}

			m_pThreadData->m_pqSearchResults->push( pTorrentSearchResult );
			WSASetEvent( m_pThreadData->m_aReservedEvents[2] );
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
		char *pLogMsg = new char[64];
		sprintf( pLogMsg, "Repairing connection status of module %u", m_nMod );
		m_pManager->LogMsg( pLogMsg );

		// lock the critical section
		CSingleLock singleLock( m_pCriticalSection, true );

		// set the repair event
		WSASetEvent( m_pThreadData->m_aReservedEvents[3] );
		m_pThreadData->m_bRepair = true;
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
	char *pLogMsg = new char[64];
	sprintf( pLogMsg, "Module %d connections repaired: %d connections", m_nMod, nCurrentConnections );
	m_pManager->LogMsg( pLogMsg );
}

//
// 
//
void ConnectionModule::TimerFired(void)
{
	// IF there is no critical section, what the hell are we doing here?
	if( m_pCriticalSection != NULL )
	{
		// lock the critical section
		CSingleLock singleLock( m_pCriticalSection, true );

		// set the repair event
		WSASetEvent( m_pThreadData->m_aReservedEvents[3] );
		m_pThreadData->m_bTimer = true;
	}
}
