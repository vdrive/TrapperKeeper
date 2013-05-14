// WAsyncSocket.cpp

#include "stdafx.h"
#include "WAsyncSocket.h"

//
//
//
WAsyncSocket::WAsyncSocket()
{
}

//
//
//
WAsyncSocket::~WAsyncSocket()
{
}

//
//
//
int WAsyncSocket::AsyncSelect(long events)
{
	int ret;

	if(events==0)
	{
		ret=WSAAsyncSelect(m_hSocket,m_hwnd,0,0);
	}
	else
	{
		ret=WSAAsyncSelect(m_hSocket,m_hwnd,WM_SOCKET_MESSAGE,events);
	}

	return ret;
}

//
//
//
int WAsyncSocket::Create(unsigned int port)
{
	int ret=WSocket::Create(port);

	if((ret!=INVALID_SOCKET)&&(ret!=SOCKET_ERROR))
	{
		// Create message window so that it will accept messages posted to it
		m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL);
		m_wnd.InitSocket(this);
		m_hwnd=m_wnd.GetSafeHwnd();
		
		// Do the asyncselect crap
		ret=AsyncSelect(FD_ALL_EVENTS);
	}

	return ret;
}

//
//
//
int WAsyncSocket::Connect(char *host,unsigned int port)
{
	m_port=port;
	WSAAsyncGetHostByName(m_hwnd,WM_GOT_HOST_BY_NAME_MESSAGE,host,m_hostent_buf,sizeof(m_hostent_buf));
	return 0;
/*
	// Check to see if we need to get the ip address from the host address
	bool is_ip_string=false;
	unsigned int ip_int[4];
	if(sscanf(host,"%u.%u.%u.%u",&ip_int[0],&ip_int[1],&ip_int[2],&ip_int[3])==4)
	{
		is_ip_string=true;
	}

	// If this is not an ip string we are connecting to, then resolve the hosts ip address from the DNS
	if(is_ip_string==false)
	{
		WSAAsyncGetHostByName(m_hwnd,WM_GOT_HOST_BY_NAME_MESSAGE,host,m_hostent_buf,sizeof(m_hostent_buf));
	}
	else	// it is just an IP
	{
		unsigned long ip_addr=inet_addr(host);
		WSAAsyncGetHostByAddr(m_hwnd,WM_GOT_HOST_BY_ADDRESS_MESSAGE,(char *)&ip_addr,sizeof(unsigned long),AF_INET,
			m_hostent_buf,sizeof(m_hostent_buf));
	}
*/
}

//
//
//
int WAsyncSocket::Close()
{
	AsyncSelect(0);	// turn off all async notifications
	
	m_wnd.DestroyWindow();
	
	return WSocket::Close();
}

//
// Message Window Functions
//

//
//
//
void WAsyncSocket::GotHostByNameMessage(WPARAM wparam,LPARAM lparam)
{
	int error_code=WSAGETASYNCERROR(lparam);

	if(error_code!=0)
	{
		OnClose(0);
		return;
	}
	
	sockaddr_in addr;
	memset(&addr,0,sizeof(sockaddr_in));
	addr.sin_family=AF_INET;
	hostent *ptr=(HOSTENT *)m_hostent_buf;
	addr.sin_addr.S_un.S_addr=*((unsigned long *)ptr->h_addr_list[0]);
	addr.sin_port=htons(m_port);

	// Connect the socket to the listening server socket
	int ret=WSAConnect(m_hSocket,(sockaddr *)&addr,sizeof(sockaddr_in),NULL,NULL,NULL,NULL);
	
	if(ret==SOCKET_ERROR)
	{
		int error=GetLastError();

		if(error!=WSAEWOULDBLOCK)
		{
			char buf[256];
			sprintf(buf,"WSAConnect() error %u in WAsyncSocket::GotHostByNameMessage()\n",error);
			OutputDebugString(buf);
		}
	}
}
/*
//
//
//
void WAsyncSocket::GotHostByAddressMessage(WPARAM wparam,LPARAM lparam)
{
	GotHostByNameMessage(wparam,lparam);
}
*/
//
//
//
void WAsyncSocket::SocketMessage(WPARAM wparam,LPARAM lparam)
{
	// wparam is the socket number, lparam is the event code (lower word) and error code (upper word)
	int event_code=WSAGETSELECTEVENT(lparam);	// lower word of lparam
	int error_code=WSAGETSELECTERROR(lparam);	// upper word of lparam

	switch(event_code)
	{
		case FD_CONNECT:
		{
			OnConnect(error_code);
			break;
		}
		case FD_ACCEPT:
		{
			OnAccept(error_code);
			break;
		}
		case FD_READ:
		{
			OnReceive(error_code);
			break;
		}
		case FD_WRITE:
		{
			OnSend(error_code);
			break;
		}
		case FD_CLOSE:
		{
			OnClose(error_code);
			break;
		}
		default:
		{
			// Unknown Socket Message
			break;
		}
	}
}

//
// Message-Based Functions
//
void WAsyncSocket::OnConnect(int error_code){}
void WAsyncSocket::OnReceive(int error_code){}
void WAsyncSocket::OnAccept(int error_code){}
void WAsyncSocket::OnClose(int error_code)
{	
	Close(); 
}
