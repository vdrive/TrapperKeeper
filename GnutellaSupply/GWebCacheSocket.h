// GWebCacheSocket.h
#pragma once

#include "TAsyncSocket.h"

class GWebCache;

class GWebCacheSocket : public TAsyncSocket
{
public:
	GWebCacheSocket();
	~GWebCacheSocket();
	void InitParent(GWebCache *parent);

	void SocketDataSent(unsigned int len);
	void SocketDataReceived(char *data,unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);

	void OnConnect(int error_code);
	void OnReceive(int error_code);

	void GetURLFile(char *url);
	void GetHostFile(char *url);

	int Close();
	
	string m_url;
private:
	GWebCache *p_parent;

	bool m_receiving_content;
};