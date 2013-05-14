// WEventSocket2.cpp

#include "stdafx.h"
#include "WEventSocket2.h"

//
//
//
WEventSocket2::WEventSocket2()
{
	m_hEvent=WSACreateEvent();
}

//
//
//
WEventSocket2::~WEventSocket2()
{
	WSACloseEvent(m_hEvent);
}

//
//
//
int WEventSocket2::EventSelect(long events)
{
	return WSAEventSelect(m_hSocket,m_hEvent,events);
}

//
//
//
WSAEVENT WEventSocket2::ReturnEventHandle()
{
	return m_hEvent;
}

//
//
//
WSANETWORKEVENTS WEventSocket2::ReturnNetworkEvents()
{
	WSANETWORKEVENTS events;	// what the fired event data was
	memset(&events,0,sizeof(WSANETWORKEVENTS));
	WSAEnumNetworkEvents(m_hSocket,m_hEvent,&events);	// reset event to not set

	return events;
}

//
//
//
int WEventSocket2::Create(unsigned int port)
{
	int ret=WSocket2::Create(port);

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
int WEventSocket2::Connect(char *host,unsigned int port)
{
	// Check to see if we need to get the ip address from the host address
	bool is_ip_string=false;
	unsigned int ip_int[4];
	if(sscanf(host,"%u.%u.%u.%u",&ip_int[0],&ip_int[1],&ip_int[2],&ip_int[3])==4)
	{
		is_ip_string=true;
	}
	
	HOSTENT *hostent=NULL;

	// If this is not an ip string we are connecting to, then resolve the hosts ip address from the DNS
	unsigned long gethostbyaddr_kludge_ip;
	if(is_ip_string==false)
	{
		hostent=gethostbyname(host);
	}
	else	// it is just an IP
	{
		// gethostbyaddr() does not seem to work properly.  We'll just get around it with a KLUDGE!
		gethostbyaddr_kludge_ip=inet_addr(host);
//		hostent=gethostbyaddr((char *)&ip,sizeof(ip),AF_INET);
	}

	// Check for error
	if((hostent==NULL) && (is_ip_string==false))
	{
		int error=GetLastError();
		OnClose(0);
		return SOCKET_ERROR;
	}

	// Connect to the host
	sockaddr_in addr;
	memset(&addr,0,sizeof(sockaddr_in));
	addr.sin_family=AF_INET;
	
	if(is_ip_string==false)
	{
		addr.sin_addr.S_un.S_addr=*((unsigned long *)hostent->h_addr_list[0]);
	}
	else
	{
		addr.sin_addr.S_un.S_addr=gethostbyaddr_kludge_ip;
	}

	addr.sin_port=htons(port);

	// Connect the socket to the listening server socket
	return WSAConnect(m_hSocket,(sockaddr *)&addr,sizeof(sockaddr_in),NULL,NULL,NULL,NULL);
}

//
//
//
int WEventSocket2::Close()
{
	EventSelect(0);	// turn off all event notifications
	return WSocket2::Close();
}

//
//
//
void WEventSocket2::OnConnect(int error_code){}
void WEventSocket2::OnReceive(int error_code){}
void WEventSocket2::OnAccept(int error_code){}
void WEventSocket2::OnClose(int error_code){}