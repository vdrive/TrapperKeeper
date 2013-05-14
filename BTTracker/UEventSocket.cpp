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
//
//
void UEventSocket::Attach(SOCKET hSocket)
{
	WEventSocket::Attach(hSocket);
	USocket::Attach(hSocket);	// not really necessary, but I just like being thorough :)
}

//
//
//
SOCKET UEventSocket::Detach()
{
	// I have to do the WEventSocket::Detach() first so that it will EventSelect(0) the m_hSocket. USocket::Detach() only calls WSocket::Detach().
	SOCKET hSocket=WEventSocket::Detach();
	USocket::Detach();	// will return an INVALID_SOCKET
	return hSocket;
}

//
// Virtual Functions
//
bool UEventSocket::OnReceive(int error_code)
{
	return false;
}
