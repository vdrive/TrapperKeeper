// TAsyncSocket.cpp

#include "stdafx.h"
#include "TAsyncSocket.h"

//
//
//
int TAsyncSocket::Create(unsigned short int port,unsigned int ip)
{
	if((WAsyncSocket::Create(port,ip)==SOCKET_ERROR) || (TSocket::Create(port,ip)==SOCKET_ERROR))
	{
		return SOCKET_ERROR;
	}
	else 
	{
		return 0;
	}
}

//
//
//
int TAsyncSocket::Close()
{
	// I have to do the WAsyncSocket::Close() first so that it will AsyncSelect(0) the m_hSocket.  TSocket::Close() just frees memory.
	if((WAsyncSocket::Close()==SOCKET_ERROR) || (TSocket::Close()==SOCKET_ERROR))
	{
		return SOCKET_ERROR;
	}
	else 
	{
		return 0;
	}
}

//
//
//
void TAsyncSocket::OnSend(int error_code)
{
	TSocket::OnSend(error_code);
}