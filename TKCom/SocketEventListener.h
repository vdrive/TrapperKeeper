#pragma once

class SocketEventListener
{
public:
	SocketEventListener(void);
	~SocketEventListener(void);
	virtual void OnClose(CAsyncSocket* src);
	virtual void OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection);
	virtual void OnSend(CAsyncSocket* src);
	virtual void OnReceive(CAsyncSocket* src,byte *data,UINT length);
	virtual void OnConnect(CAsyncSocket* src);
	virtual void OnConnectFailed(CAsyncSocket* src);
};
