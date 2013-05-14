// WAsyncSocket.cpp

#include "stdafx.h"
#include "WAsyncSocket.h"


//
//
//
WAsyncSocket::WAsyncSocket()
{
	// Create message window so that it will accept messages posted to it
	//m_wnd.CreateEx(0,AfxRegisterWndClass(0),NULL,0,0,0,0,0,0,NULL,NULL);
	//m_wnd.InitSocket(this);
	//m_hwnd=m_wnd.GetSafeHwnd();
}

//
//
//
WAsyncSocket::~WAsyncSocket()
{
	//AsyncSelect(0);	// turn off all async notifications
	//m_wnd.DestroyWindow();
}

//
//
//
int WAsyncSocket::AsyncSelect(long events)
{
	if(events==0)
	{
		return WSAAsyncSelect(m_hSocket,m_hwnd,0,0);
	}
	else
	{
		return WSAAsyncSelect(m_hSocket,m_hwnd,WM_SOCKET_MESSAGE,events);
	}
}

//
//
//
int WAsyncSocket::Create(unsigned short int port,unsigned int ip)
{
	int ret=WSocket::Create(port,ip);

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
int WAsyncSocket::Connect(const char *host,unsigned short int port)
{
	m_port=port;
	WSAAsyncGetHostByName(m_hwnd,WM_GOT_HOST_BY_NAME_MESSAGE,host,m_hostent_buf,sizeof(m_hostent_buf));
	return 0;
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
void WAsyncSocket::GotHostByNameMessage(WPARAM wparam,LPARAM lparam)
{
int error_code=WSAGETASYNCERROR(lparam);

	if(error_code!=0)
	{
		Close();
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
#ifdef _DEBUG
			char buf[128];
			sprintf(buf,"WSAConnect() error %u in WAsyncSocket::GotHostByNameMessage()\n",error);
			OutputDebugString(buf);
#endif // _DEBUG
		}
	}
}

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
// Virtual Functions
//
void WAsyncSocket::OnConnect(int error_code){}
void WAsyncSocket::OnReceive(int error_code){}
void WAsyncSocket::OnSend(int error_code){}
void WAsyncSocket::OnAccept(int error_code){}
void WAsyncSocket::OnClose(int error_code)
{
	Close();
}