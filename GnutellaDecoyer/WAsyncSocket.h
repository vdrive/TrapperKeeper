// WAsyncSocket.h
#pragma once

#include "WSocket.h"
#include "WAsyncSocketWnd.h"

class WAsyncSocket : public WSocket
{
public:
	WAsyncSocket();
	~WAsyncSocket();

	virtual int Create(unsigned int port);
	virtual int Close();

	int Connect(char *host,unsigned int port);

	virtual void OnConnect(int error_code);
	virtual void OnReceive(int error_code);
	virtual void OnAccept(int error_code);
	virtual void OnClose(int error_code);

	// Message Window Functions
	void GotHostByNameMessage(WPARAM wparam,LPARAM lparam);
//	void GotHostByAddressMessage(WPARAM wparam,LPARAM lparam);
	void SocketMessage(WPARAM wparam,LPARAM lparam);

protected:
	int AsyncSelect(long events);

	// Protected Data Members
	WAsyncSocketWnd m_wnd;
	HWND m_hwnd;

	// Variable for WSAAsyncGetHostByName and WSAAsyncGetHostByAddr
	char m_hostent_buf[MAXGETHOSTSTRUCT];
	unsigned int m_port;
};