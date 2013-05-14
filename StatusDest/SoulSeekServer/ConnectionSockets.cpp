// ConnectionSockets.cpp

#include "stdafx.h"
#include "ConnectionSockets.h"

//
//
//
ConnectionSockets::ConnectionSockets(HWND hwnd)
{
	m_hwnd=hwnd;

	// Init sockets
	for(unsigned int i=0;i<WSA_MAXIMUM_WAIT_EVENTS-4;i++)
	{
		m_sockets[i].InitParent(this);
	}

	// Create the reserved events
	for(i=0;i<4;i++)
	{
		m_events[i]=WSACreateEvent();
		if(m_events[i]==WSA_INVALID_EVENT)
		{
			::MessageBox(NULL,"Could not create event in ConnectionSockets()","Error",MB_OK);
		}
	}

	// Fully initialize the events array
	for(i=0;i<WSA_MAXIMUM_WAIT_EVENTS-4;i++)
	{
		m_events[4+i]=m_sockets[i].ReturnEventHandle();
	}
}

//
//
//
ConnectionSockets::~ConnectionSockets()
{
	// Close the reserved events
	for(unsigned int i=0;i<4;i++)
	{
		if(WSACloseEvent(m_events[i])==FALSE)
		{
			::MessageBox(NULL,"Could not close event in ~ConnectionSockets()","Error",MB_OK);
		}
	}
}

//
//
//
int ConnectionSockets::AddParentConnection(SOCKET socket,char* ip)
{
	for( int i = 0; i < 60; i++ )
	{
		if( !m_sockets[i].IsSocket() )
		{
			m_sockets[i].Attach( socket, ip );
			return 0;
		}
	}

	TRACE( "SOMETHING BAD HAPPENED!\n" );

	closesocket( socket );

	return -1;
}