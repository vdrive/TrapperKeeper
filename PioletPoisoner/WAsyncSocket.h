// WAsyncSocket.h

#ifndef W_ASYNC_SOCKET_H
#define W_ASYNC_SOCKET_H

#include "WSocket.h"
#include "WAsyncSocketWnd.h"

class WAsyncSocket : public WSocket
{
public:
	WAsyncSocket();
	~WAsyncSocket();

	int Create(unsigned int port=0);
	virtual int Create(unsigned short int port,unsigned int ip=INADDR_ANY);
	int Connect(char *host,unsigned int port);
	virtual int Close();

	// Message Window Functions
	void GotHostByNameMessage(WPARAM wparam,LPARAM lparam);
	void GotHostByAddressMessage(WPARAM wparam,LPARAM lparam);
	void SocketMessage(WPARAM wparam,LPARAM lparam);

protected:
	int AsyncSelect(long events);

	virtual void OnConnect(int error_code);
	virtual void OnReceive(int error_code);
	virtual void OnAccept(int error_code);
	virtual void OnClose(int error_code);

	// Protected Data Members
	WAsyncSocketWnd m_wnd;
	HWND m_hwnd;

	// Variable for WSAAsyncGetHostByName and WSAAsyncGetHostByAddr
	char m_hostent_buf[MAXGETHOSTSTRUCT];
	unsigned int m_port;
};

#endif // W_ASYNC_SOCKET_H