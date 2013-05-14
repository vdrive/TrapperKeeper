// ConnectionSockets.cpp

#include "StdAfx.h"
#include "ConnectionSockets.h"
#include "ConnectionModuleMessageWnd.h"
#include "TorrentIndexResult.h"
#include "iphlpapi.h"

//
//
//
ConnectionSockets::ConnectionSockets()
{
	m_dlg_hwnd=NULL;
	int num_reserved_events=ReturnNumberOfReservedEvents();
	int num_socket_events=ReturnNumberOfSocketEvents();

	// Init the parent pointers and message window handles
	for( int i = 0; i < num_socket_events; i++ )
	{
		m_sockets[i].InitParent(this);
		m_sockets[i].InitIndex(i);
	}
	
	// Create the reserved events
	for(i=0;i<num_reserved_events;i++)
	{
		m_events[i]=WSACreateEvent();
		if(m_events[i]==WSA_INVALID_EVENT)
		{
			char msg[1024];
			strcpy(msg,"Could not create a valid reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}

	// Fully initialize events array
	for( i = 0; i < num_socket_events; i++ )
	{
		m_events[num_reserved_events+i]=m_sockets[i].ReturnEventHandle();
	}

}

//
//
//
ConnectionSockets::~ConnectionSockets()
{
	int num_reserved_events=ReturnNumberOfReservedEvents();
	
	// Close the reserved events
	for(int i=0;i<num_reserved_events;i++)
	{
		if(WSACloseEvent(m_events[i])==false)
		{
			char msg[1024];
			strcpy(msg,"Could not close reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}
	// close all sockets
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].Destroy();
	}
}

//
//
//
void ConnectionSockets::AddNewClient(SOCKET hSocket)
{
	bool found = false;
	for( int i = 0; i < NumberOfSocketEvents; i++ )
	{
		if( m_sockets[i].IsSocket() == false )
		{
			found = true;
			m_sockets[i].Attach( hSocket );
			break;
		}
	}

	if( !found )
	{
		::PostMessage( m_dlg_hwnd, WM_CMOD_CLIENT_CONNECT_ERROR, NULL, NULL );
		closesocket( hSocket );	
	}
}

//
//
//
void ConnectionSockets::KillClient(const IPPort &rIP)
{
	for( int i = 0; i < NumberOfSocketEvents; i++ )
	{
		if( m_sockets[i].GetClientIP() == rIP )
		{
			m_sockets[i].Close();
			break;
		}
	}
}

//
//
//
void ConnectionSockets::TorrentIndexRes(TorrentIndexResult *pTorrentIndexResponse)
{
	if( pTorrentIndexResponse != NULL )
	{
		if( m_sockets[pTorrentIndexResponse->m_nClient].GetClientIP() == pTorrentIndexResponse->m_client )
		{
			m_sockets[pTorrentIndexResponse->m_nClient].TorrentIndexRes( pTorrentIndexResponse );
		}
		else
		{
			delete pTorrentIndexResponse;
			pTorrentIndexResponse = NULL;
		}
	}
}

//
//
//
void ConnectionSockets::RepairConnections(void)
{
	int nCurrentConnections = 0;

	// FOR each of the sockets
	for( int i = 0; i < NumberOfSocketEvents; i++ )
	{
		// IF the socket is unconnected
		if( m_sockets[i].IsSocket() == false )
		{
			// reinitialize it
			m_sockets[i].Close();
		}
		// ELSE the socket is connected
		else
		{
			// increment the total number of socket connections
			nCurrentConnections++;
		}
	}

	// return new counts for socket status
	::PostMessage( m_dlg_hwnd, WM_CMOD_CONNECTIONS_REPAIRED, (WPARAM)nCurrentConnections, NULL );
}
