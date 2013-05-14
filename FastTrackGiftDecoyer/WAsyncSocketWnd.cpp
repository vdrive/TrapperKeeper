// WAsyncSocketWnd.cpp

#include "stdafx.h"
#include "WAsyncSocketWnd.h"
#include "WAsyncSocket.h"

BEGIN_MESSAGE_MAP(WAsyncSocketWnd,CWnd)
	ON_MESSAGE(WM_GOT_HOST_BY_NAME_MESSAGE,GotHostByNameMessage)
	ON_MESSAGE(WM_SOCKET_MESSAGE,SocketMessage)
END_MESSAGE_MAP()

//
//
//
WAsyncSocketWnd::WAsyncSocketWnd()
{
	p_socket=NULL;
}

//
//
//
WAsyncSocketWnd::~WAsyncSocketWnd()
{
}

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
LRESULT WAsyncSocketWnd::SocketMessage(WPARAM wparam,LPARAM lparam)
{
	p_socket->SocketMessage(wparam,lparam);
	return 0;
}
