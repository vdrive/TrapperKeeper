// WAsyncSocketWnd.h

#ifndef WASYNC_SOCKET_WND_H
#define WASYNC_SOCKET_WND_H

#define WM_GOT_HOST_BY_NAME_MESSAGE		WM_USER+1
#define WM_SOCKET_MESSAGE				WM_USER+2

class WAsyncSocket;

class WAsyncSocketWnd : public CWnd
{
public:
	WAsyncSocketWnd();
	~WAsyncSocketWnd();
	void InitSocket(WAsyncSocket *socket);

protected:
	WAsyncSocket *p_socket;

	LRESULT GotHostByNameMessage(WPARAM wparam,LPARAM lparam);
	LRESULT SocketMessage(WPARAM wparam,LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};

#endif // W_ASYNC_SOCKET_WND_H