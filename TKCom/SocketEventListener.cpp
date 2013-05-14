#include "StdAfx.h"
#include "socketeventlistener.h"

SocketEventListener::SocketEventListener(void)
{
}

SocketEventListener::~SocketEventListener(void)
{
}

void SocketEventListener::OnClose(CAsyncSocket* src)
{
}

void SocketEventListener::OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection)
{
	delete new_connection;
}

void SocketEventListener::OnSend(CAsyncSocket* src)
{
}

void SocketEventListener::OnReceive(CAsyncSocket* src,byte *data,UINT length)
{
}

void SocketEventListener::OnConnect(CAsyncSocket* src)
{
}

void SocketEventListener::OnConnectFailed(CAsyncSocket* src)
{
}