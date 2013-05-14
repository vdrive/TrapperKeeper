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

//
//
//
int UAsyncSocket::Attach(SOCKET hSocket)
{
	int ret=0;
	ret=WAsyncSocket::Attach(hSocket);
	if(ret!=0)
	{
		return ret;
	}
	return USocket::Attach(hSocket);	// not really necessary, but I just like being thorough :)
}

//
//
//
SOCKET UAsyncSocket::Detach()
{
	// I have to do the WAsyncSocket::Detach() first so that it will AsyncSelect(0) the m_hSocket. USocket::Detach() only calls WSocket::Detach().
	SOCKET hSocket=WAsyncSocket::Detach();
	USocket::Detach();	// will return an INVALID_SOCKET
	return hSocket;
}

//
// Virtual Functions
//
bool UAsyncSocket::OnReceive(int error_code)
{
	return false;
}
