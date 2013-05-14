//
// CClientReceiver.cpp
//
#include "StdAfx.h"
#include "ClientReceiver.h"
#include "ClientReceiverThreadData.h"
#include "BTDecoyClientDll.h"
#include "AcceptedSocket.h"
#include "IP.h"
#include <afxmt.h>	// for CCriticalSection

BEGIN_MESSAGE_MAP(CClientReceiver, CWnd)
	ON_MESSAGE(WM_LISTENER_INIT_THREAD_DATA, InitThreadData)
	ON_MESSAGE(WM_LISTENER_NEW_SOCKET, NewConnection)
	ON_MESSAGE(WM_LISTENER_STATUS, CurrentStatus)
	ON_MESSAGE(WM_LISTENER_LOG_MSG, LogMsg)
END_MESSAGE_MAP()

//
//
//
UINT ClientReceiverThreadProc(LPVOID pParam)
{
	// Init message window handle
	if( pParam == NULL )
		return (UINT)(-1);

	HWND hwnd = (HWND)pParam;

	// Create the listening socket, but do not open it
	ListeningSocket listener( hwnd );
	listener.Open();

	// Link event into events array
	WSAEVENT aEvents[CLIENTRECEIVER_RESERVED_EVENTS+1];
	for( int i = 0; i < CLIENTRECEIVER_RESERVED_EVENTS; i++ )
	{
		aEvents[i] = WSACreateEvent();
		if( aEvents[i] == WSA_INVALID_EVENT )
			return (UINT)-1;
	}
	aEvents[CLIENTRECEIVER_RESERVED_EVENTS] = listener.ReturnEventHandle();

	// Threaddata variables
	ClientReceiverThreadData threadData;
	memcpy( threadData.m_aReservedEvents, aEvents, sizeof(WSAEVENT) * CLIENTRECEIVER_RESERVED_EVENTS );
	CCriticalSection criticalSection;

	// request initialization of the thread data
	::PostMessage( hwnd, WM_LISTENER_INIT_THREAD_DATA, (WPARAM)&criticalSection, (LPARAM)&threadData );

	WSANETWORKEVENTS events;			// what the fired event data was
	DWORD event;						// which event fired
	char *pLogMsg = NULL;
	
	while( true )
	{
		event = WSAWaitForMultipleEvents( (CLIENTRECEIVER_RESERVED_EVENTS + 1), aEvents, false, WSA_INFINITE, false );
		
		char buf[16];
		_itoa( event, buf, 10 );

		if( event == WSA_WAIT_FAILED )
			AfxAbort();

		// exit event
		if( event == 0 )
		{
			CSingleLock singleLock( &criticalSection, true );
			WSAResetEvent( aEvents[event] );
			break;
		}

		// change status
		if( event == 1 )
		{
			CSingleLock singleLock( &criticalSection, true );

			switch( threadData.m_eStatus )
			{
			case ListeningSocket::OPEN:
				listener.Open();
				break;
			case ListeningSocket::CLOSED:
				listener.Close();
				break;
			}

			WSAResetEvent( aEvents[event] );
		}

		// socket event
		if( event >= CLIENTRECEIVER_RESERVED_EVENTS )
		{
			WSAResetEvent( aEvents[event] );

			events = listener.ReturnNetworkEvents();
			
			if(events.lNetworkEvents & FD_CONNECT)
			{
				listener.OnConnect(events.iErrorCode[FD_CONNECT_BIT]);
			}
			if(events.lNetworkEvents & FD_READ)
			{
				while( listener.OnReceive(events.iErrorCode[FD_READ_BIT]) );
			}
			if(events.lNetworkEvents & FD_WRITE)
			{
				listener.OnSend(events.iErrorCode[FD_WRITE_BIT]);
			}
			if(events.lNetworkEvents & FD_CLOSE)
			{
				listener.OnClose(events.iErrorCode[FD_CLOSE_BIT]);
			}
			if(events.lNetworkEvents & FD_ACCEPT)
			{
				listener.OnAccept(events.iErrorCode[FD_ACCEPT_BIT]);
			}
		}

	}	// end while(1)

	// Close the reserved events
	for( int i = 0; i < CLIENTRECEIVER_RESERVED_EVENTS; i++ )
		WSACloseEvent( aEvents[i] );

	// close the socket
	listener.Close();

	return 0;	// exit the thread
}

//
//
//
CClientReceiver::CClientReceiver(void)
: m_pParent(NULL), m_pCriticalSection(NULL), m_pThreadData(NULL), m_pThread(NULL)
, m_eStatus(ListeningSocket::ListenerStatus::CLOSED)
{
}

//
//
//
CClientReceiver::~CClientReceiver()
{
	Stop();
}

//
//
//
void CClientReceiver::Run(void)
{
	// Create message window so that it will accept messages posted to it
	if( this->CreateEx( 0, AfxRegisterWndClass(0), NULL, 0, 0, 0, 0, 0, 0, NULL, NULL ) == false )
	{
		m_pParent->LogMsg( "CClientReceiver failed to generate window, listener will not open" );
		return;
	}

	m_pThread = AfxBeginThread( ClientReceiverThreadProc, (LPVOID)this->GetSafeHwnd(), THREAD_PRIORITY_BELOW_NORMAL );
}

//
//
//
void CClientReceiver::Stop(void)
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
		WaitForSingleObject( m_pThread->m_hThread, 2000 );
		m_pThread = NULL;
	}

	// Destroy message window
	this->DestroyWindow();
}

//
//
//
void CClientReceiver::SetOpen(void)
{
	if( m_pCriticalSection != NULL && m_pThreadData != NULL )
	{
		CSingleLock singleLock( m_pCriticalSection, true );

		m_pThreadData->m_eStatus = ListeningSocket::ListenerStatus::OPEN;

		WSASetEvent( m_pThreadData->m_aReservedEvents[1] );
	}
}

//
//
//
void CClientReceiver::SetClosed(void)
{
	if( m_pCriticalSection != NULL && m_pThreadData != NULL )
	{
		CSingleLock singleLock( m_pCriticalSection, true );

		m_pThreadData->m_eStatus = ListeningSocket::ListenerStatus::CLOSED;

		WSASetEvent( m_pThreadData->m_aReservedEvents[1] );
	}
}

//
//
//
LRESULT CClientReceiver::InitThreadData(WPARAM wparam, LPARAM lparam)
{
	m_pCriticalSection = (CCriticalSection *)wparam;
	m_pThreadData = (ClientReceiverThreadData *)lparam;

	return 0;
}

//
//
//
LRESULT CClientReceiver::NewConnection(WPARAM wparam, LPARAM lparam)
{
	m_pParent->AddNewClient( (const AcceptedSocket *)wparam );

	return 0;
}

//
//
//
LRESULT CClientReceiver::CurrentStatus(WPARAM wparam, LPARAM lparam)
{
	m_eStatus = (ListeningSocket::ListenerStatus)wparam;

	return 0;
}

//
//
//
LRESULT CClientReceiver::LogMsg(WPARAM wparam, LPARAM lparam)
{
	if( wparam != NULL )
	{
		m_pParent->LogMsg( (char *)wparam );
		delete (char *)wparam;
	}

	return 0;
}
