// WAsyncSocket2Wnd.cpp

#include "stdafx.h"
#include "WAsyncSocket2Wnd.h"
#include "WAsyncSocket2.h"

BEGIN_MESSAGE_MAP(WAsyncSocket2Wnd,CWnd)
	ON_MESSAGE(WM_GOT_HOST_BY_NAME_MESSAGE,GotHostByNameMessage)
	ON_MESSAGE(WM_GOT_HOST_BY_ADDRESS_MESSAGE,GotHostByAddressMessage)
	ON_MESSAGE(WM_SOCKET_MESSAGE,SocketMessage)
END_MESSAGE_MAP()

//
//
//
void WAsyncSocket2Wnd::InitSocket(WAsyncSocket2 *socket)
{
	p_socket=socket;
}

//
//
//
LRESULT WAsyncSocket2Wnd::GotHostByNameMessage(WPARAM wparam,LPARAM lparam)
{
	p_socket->GotHostByNameMessage(wparam,lparam);
	return 0;
}

//
//
//
LRESULT WAsyncSocket2Wnd::GotHostByAddressMessage(WPARAM wparam,LPARAM lparam)
{
	p_socket->GotHostByAddressMessage(wparam,lparam);
	return 0;
}

//
//
//
LRESULT WAsyncSocket2Wnd::SocketMessage(WPARAM wparam,LPARAM lparam)
{
	p_socket->SocketMessage(wparam,lparam);
	return 0;
}