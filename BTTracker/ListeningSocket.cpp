#include "StdAfx.h"
#include "ListeningSocket.h"
#include "ClientReceiver.h"

//
//
//
ListeningSocket::ListeningSocket(HWND hwnd)
: m_hwnd(hwnd), m_eStatus(ListenerStatus::CLOSED)
{
}

//
//
//
ListeningSocket::~ListeningSocket(void)
{
	Close();
}

//
// We're listening, so OnAccept we should do some stuff
//
void ListeningSocket::OnAccept(int errorcode)
{
	if( errorcode != 0 )
		return;

	// Accept the new socket
	sockaddr_in sinRemote;
	int nAddrSize = sizeof( sinRemote );
	ZeroMemory( &sinRemote, nAddrSize );
	SOCKET hSocket = accept( m_hSocket, (sockaddr *)&sinRemote, &nAddrSize );

	// IF the connection failed
	if( hSocket == INVALID_SOCKET && GetLastError() != WSAEWOULDBLOCK )
	{
		return;
	}
	else
	{
		// Send the new socket back to the main window to be stored
		::PostMessage( m_hwnd, WM_LISTENER_NEW_SOCKET, (WPARAM)hSocket, NULL );
	}
}

//
//
//
HANDLE ListeningSocket::Open(void)
{
	if( m_eStatus == CLOSED )
	{
		if( this->Create( TCPSERVERPORT, INADDR_ANY ) != 0 || this->Listen() != 0 )
		{
			SetStatusClosed();
			return NULL;
		}
		else
		{
			SetStatusOpen();
		}
	}

	::PostMessage( m_hwnd, WM_LISTENER_STATUS, (WPARAM)m_eStatus, NULL );
	return ReturnEventHandle();
}

//
//
//
int ListeningSocket::Close(void)
{
	if( m_eStatus != CLOSED )
	{
		SetStatusClosed();

		::PostMessage( m_hwnd, WM_LISTENER_STATUS, (WPARAM)m_eStatus, NULL );

		return ( __super::Close() );
	}
	else
		return 0;
}