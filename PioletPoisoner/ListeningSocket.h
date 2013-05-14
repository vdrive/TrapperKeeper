// ListeningSocket.h

#ifndef LISTENING_SOCKET_H
#define LISTENING_SOCKET_H

#include "TAsyncSocket.h"

class ConnectionModule;

class ListeningSocket : public TAsyncSocket
{
public:
	ListeningSocket();
	void InitParent(ConnectionModule *parent);
	void OnAccept(int error_code);
	int Create(unsigned int port, const char* ip, UINT filesize, const char* md5);
	void OnClose(int error_code);
	void CheckIdle();

	ConnectionModule *p_parent;
	CString m_ip;
	int m_port;
	UINT m_filesize;
	CString m_md5;
	CTime m_creation_time;
	bool m_waiting_for_connection;
};

#endif // LISTENING_SOCKET_H