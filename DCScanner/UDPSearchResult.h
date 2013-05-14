#pragma once
#include "uasyncsocket.h"

class UDPSearchResult :
	public UAsyncSocket
{
public:
	UDPSearchResult(void);
	~UDPSearchResult(void);
	void OnConnect(int error_code);
	void OnReceive(int error_code);
	int ReceiveFrom(void *data,unsigned int len,unsigned int *ip,unsigned short *port,unsigned int*num_read);
	void OnSend(int error_code);
	int SendTo(void *data,unsigned int len,unsigned int ip,unsigned short port);
	//int SendTo(void *data,unsigned int len,unsigned ip,unsigned short port,unsigned int *num_sent=NULL);
private:
};
