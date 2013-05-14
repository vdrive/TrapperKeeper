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
	WSANETWORKEVENTS events;							// what the fired event data was
	memset(&events,0,sizeof(WSANETWORKEVENTS));
	WSAEnumNetworkEvents(m_hSocket,m_hEvent,&events);	// reset event to not set
	return events;
}

//
//
//
int WEventSocket::Create(unsigned short int port,unsigned int ip)
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
int WEventSocket::Connect(const char *host,unsigned short int port)
{
	HOSTENT *hostent=NULL;
	hostent=gethostbyname(host);

	// Check for error
	if(hostent==NULL)
	{
		Close();
		return SOCKET_ERROR;
	}

	sockaddr_in addr;
	memset(&addr,0,sizeof(sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=*((unsigned long *)hostent->h_addr_list[0]);
	addr.sin_port=htons(port);

	// Connect the socket to the listening server socket
	int ret=WSAConnect(m_hSocket,(sockaddr *)&addr,sizeof(sockaddr_in),NULL,NULL,NULL,NULL);

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
// Virtual Functions
//
void WEventSocket::OnConnect(int error_code){}
void WEventSocket::OnReceive(int error_code){}
void WEventSocket::OnSend(int error_code){}
void WEventSocket::OnAccept(int error_code){}
void WEventSocket::OnClose(int error_code)
{
	Close();
}