// HttpSocket.h

#ifndef HTTP_SOCKET_H
#define HTTP_SOCKET_H

#include "TAsyncSocket.h"

class SupernodeDistributerDll;

class HttpSocket : public TAsyncSocket
{
public:
	HttpSocket();
	~HttpSocket();
	void InitParent(SupernodeDistributerDll *parent);

	int Connect(char *host,unsigned short int port,CTime start_time);

	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);

	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void TimeOut();
	CTime m_start_time;

private:
	SupernodeDistributerDll *p_parent;

	char m_ip[15+1];
	unsigned short int m_port;
};

#endif // HTTP_SOCKET_H