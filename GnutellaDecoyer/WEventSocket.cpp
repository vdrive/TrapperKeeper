// WEventSocket.cpp

#include "stdafx.h"
#include "WEventSocket.h"

//
//
//
WEventSocket::WEventSocket()
{
	m_hEvent=WSACreateEvent();
}

//
//
//
WEventSocket::~WEventSocket()
{
	WSACloseEvent(m_hEvent);
}

//
//
//
int WEventSocket::EventSelect(long events)
{
	return WSAEventSelect(m_hSocket,m_hEvent,events);
}

//
//
//
WSAEVENT WEventSocket::ReturnEventHandle()
{
	return m_hEvent;
}

//
//
//
WSANETWORKEVENTS WEventSocket::ReturnNetworkEvents()
{
	WSANETWORKEVENTS events;	// what the fired event data was
	memset(&events,0,sizeof(WSANETWORKEVENTS));
	WSAEnumNetworkEvents(m_hSocket,m_hEvent,&events);	// reset event to not set

	return events;
}

//
//
//
int WEventSocket::Create(unsigned int port)
{
	int ret=WSocket::Create(port);

	if((ret!=INVALID_SOCKET)&&(ret!=SOCKET_ERROR))
	{
		// Do the event select crap
		ret=EventSelect(FD_ALL_EVENTS);
	}

	return ret;
}

//
//
//
int WEventSocket::Close()
{
	EventSelect(0);	// turn off all event notifications
	return WSocket::Close();
}

//
//
//
void WEventSocket::OnConnect(int error_code){}
void WEventSocket::OnReceive(int error_code){}
void WEventSocket::OnAccept(int error_code){}
void WEventSocket::OnClose(int error_code)
{
	Close();
}