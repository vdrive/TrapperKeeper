// TEventSocket.cpp

#include "stdafx.h"
#include "TEventSocket.h"

//
//
//
int TEventSocket::Create(unsigned short int port,unsigned int ip)
{
	return WEventSocket::Create(port,ip);
}

//
//
//
int TEventSocket::Close()
{
	if((TSocket::Close()==SOCKET_ERROR) || (WEventSocket::Close()==SOCKET_ERROR))
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
void TEventSocket::OnSend(int error_code)
{
	TSocket::OnSend(error_code);
}