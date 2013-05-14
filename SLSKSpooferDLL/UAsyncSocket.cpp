// UAsyncSocket.cpp

#include "stdafx.h"
#include "UAsyncSocket.h"

//
//
//
UAsyncSocket::UAsyncSocket()
{
}

//
//
//
UAsyncSocket::~UAsyncSocket()
{
}

//
//
//
int UAsyncSocket::Create(unsigned short int port,unsigned int ip)
{
	return WAsyncSocket::Create(port,ip);
}

//
//
//
int UAsyncSocket::Close()
{
	if((USocket::Close()==SOCKET_ERROR) || (WAsyncSocket::Close()==SOCKET_ERROR))
	{
		return SOCKET_ERROR;
	}
	else 
	{
		return 0;
	}
}