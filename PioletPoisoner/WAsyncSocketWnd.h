// WAsyncSocketWnd.h

#ifndef W_ASYNC_SOCKET_WND_H
#define W_ASYNC_SOCKET_WND_H

#define WM_GOT_HOST_BY_NAME_MESSAGE		WM_USER+1
#define WM_GOT_HOST_BY_ADDRESS_MESSAGE	WM_USER+2
#define WM_SOCKET_MESSAGE				WM_USER+3

class WAsyncSocket;

class WAsyncSocketWnd : public CWnd
{
public:
	void InitSocket(WAsyncSocket *socket);

protected:
	afx_msg LRESULT GotHostByNameMessage(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT GotHostByAddressMessage(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SocketMessage(WPARAM wparam,LPARAM lparam);

	// Protected Data Members
	WAsyncSocket *p_socket;

	DECLARE_MESSAGE_MAP()
};

#endif // W_ASYNC_SOCKET_WND_H