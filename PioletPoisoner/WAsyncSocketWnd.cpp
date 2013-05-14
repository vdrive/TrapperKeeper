// WAsyncSocketWnd.cpp

#include "stdafx.h"
#include "WAsyncSocketWnd.h"
#include "WAsyncSocket.h"

BEGIN_MESSAGE_MAP(WAsyncSocketWnd,CWnd)
	ON_MESSAGE(WM_GOT_HOST_BY_NAME_MESSAGE,GotHostByNameMessage)
	ON_MESSAGE(WM_GOT_HOST_BY_ADDRESS_MESSAGE,GotHostByAddressMessage)
	ON_MESSAGE(WM_SOCKET_MESSAGE,SocketMessage)
END_MESSAGE_MAP()

//
//
//
void WAsyncSocketWnd::InitSocket(WAsyncSocket *socket)
{
	p_socket=socket;
}

//
//
//
LRESULT WAsyncSocketWnd::GotHostByNameMessage(WPARAM wparam,LPARAM lparam)
{
	p_socket->GotHostByNameMessage(wparam,lparam);
	return 0;
}

//
//
//
LRESULT WAsyncSocketWnd::GotHostByAddressMessage(WPARAM wparam,LPARAM lparam)
{
	p_socket->GotHostByAddressMessage(wparam,lparam);
	return 0;
}

//
//
//
LRESULT WAsyncSocketWnd::SocketMessage(WPARAM wparam,LPARAM lparam)
{
	p_socket->SocketMessage(wparam,lparam);
	return 0;
}