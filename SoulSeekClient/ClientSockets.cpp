// ConnectionSockets.cpp

#include "stdafx.h"
#include "ClientSockets.h"
#include "ClientModuleMessageWnd.h"

//
//
//
ClientSockets::ClientSockets(HWND hwnd)
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
ClientSockets::~ClientSockets()
{
	// Close the reserved events
	for(unsigned int i=0;i<4;i++)
	{
		if(WSACloseEvent(m_events[i])==FALSE)
		{
			::MessageBox(NULL,"Could not close event in ~ConnectionSockets()","Error",MB_OK);
		}
	}
	for( int i = 0; i < 60; i++ )
	{
		if( m_sockets[i].IsSocket() )
			m_sockets[i].Close();
	}
}
void ClientSockets::CheckForTimeout()
{
	for( int i = 0; i < 60; i++ )
	{
		if( m_sockets[i].IsSocket() )
		{
			CTimeSpan duration(CTime::GetCurrentTime() - m_sockets[i].m_connectTime);
			if(duration.GetTotalSeconds() > 4)
			{
				//LONGLONG i = duration.GetTotalSeconds();
				m_sockets[i].Close();
			}
		}
	}
}
void ClientSockets::ClearAll()
{
	for( int i = 0; i < 60; i++ )
	{
		m_sockets[i].Close();
	}
}
//
//
//
int ClientSockets::SendPacket(PacketWriter* packet)
{
	for( int i = 0; i < 60; i++ )
	{
		if( !m_sockets[i].IsSocket() )
		{
			m_sockets[i].SendPacket(packet);
			return 0;
		}
	}
	::PostMessage(m_hwnd, WM_USER+2, (WPARAM)strdup("Error creating socket!\n"), NULL);
	delete packet;
	return -1;
}