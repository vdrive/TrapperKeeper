// UEventSocket.cpp

#include "stdafx.h"
#include "UEventSocket.h"

//
//
//
UEventSocket::UEventSocket()
{
}

//
//
//
UEventSocket::~UEventSocket()
{
}

//
//
//
int UEventSocket::Create(unsigned short int port,unsigned int ip)
{
	return WEventSocket::Create(port,ip);
}

//
//
//
int UEventSocket::Close()
{
	if((USocket::Close()==SOCKET_ERROR) || (WEventSocket::Close()==SOCKET_ERROR))
	{
		return SOCKET_ERROR;
	}
	else 
	{
		return 0;
	}
}

//
// Virtual Functions
//
void UEventSocket::OnReceive(int error_code){}
