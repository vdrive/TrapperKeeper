// WAsyncSocket.h

#ifndef WASYNC_SOCKET_H
#define WASYNC_SOCKET_H

#include "WSocket.h"
#include "WAsyncSocketWnd.h"

class WAsyncSocket : virtual public WSocket
{
public:
	virtual int Create(unsigned short int port,unsigned int ip=INADDR_ANY);
	virtual int Close();

	virtual int Attach(SOCKET hSocket);
	virtual SOCKET Detach();

	int Connect(char *host,unsigned short int port);
	
	virtual void OnConnect(int error_code);
	virtual bool OnReceive(int error_code);
	virtual void OnSend(int error_code);
	virtual void OnAccept(int error_code);
	virtual void OnClose(int error_code);

	// Message Window Functions
	void GotHostByNameMessage(WPARAM wparam,LPARAM lparam);
	void SocketMessage(WPARAM wparam,LPARAM lparam);

protected:
	int AsyncSelect(long events);

	// Protected Data Members
	WAsyncSocketWnd m_wnd;
	HWND m_hwnd;

	// Variables for WSAAsyncGetHostByName
	char m_hostent_buf[MAXGETHOSTSTRUCT];
	unsigned int m_port;
};

#endif // WASYNC_SOCKET_H