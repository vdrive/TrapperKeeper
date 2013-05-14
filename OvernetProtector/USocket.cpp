// USocket.cpp

#include "stdafx.h"
#include "USocket.h"

//
//
//
USocket::USocket()
{
	m_socket_type=SOCK_DGRAM;	// SOCK_STREAM or SOCK_DGRAM
}

//
//
//
USocket::~USocket()
{
}

//
//
//
int USocket::SendTo(void *data,unsigned int len,unsigned int ip,unsigned short int port,unsigned int *num_sent)
{
	WSABUF wsabuf;
	wsabuf.buf=(char *)data;
	wsabuf.len=len;

	DWORD sent=0;

	sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=ip;
	addr.sin_port=htons(port);

	int ret=WSASendTo(m_hSocket,&wsabuf,1,&sent,0,(sockaddr *)&addr,sizeof(sockaddr_in),NULL,NULL);

	if(num_sent!=NULL)
	{
		*num_sent=sent;
	}

	return ret;
}

//
//
//
int USocket::ReceiveFrom(void *data,unsigned int len,unsigned int *ip,unsigned short int *port,unsigned int *num_read)
{
	WSABUF wsabuf;
	wsabuf.buf=(char *)data;
	wsabuf.len=len;

	DWORD read=0;
	DWORD flags=0;
	
	sockaddr_in addr;
	memset(&addr,0,sizeof(sockaddr_in));
	int sockaddr_in_len=sizeof(sockaddr_in);

	int ret=WSARecvFrom(m_hSocket,&wsabuf,1,&read,&flags,(sockaddr *)&addr,&sockaddr_in_len,NULL,NULL);

	// If there was success, set the ip and port
	if(ret==0)
	{
		*ip=addr.sin_addr.S_un.S_addr;
		*port=ntohs(addr.sin_port);
		*num_read=read;
	}
		
	return ret;
}