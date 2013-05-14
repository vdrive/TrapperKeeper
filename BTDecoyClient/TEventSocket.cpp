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
	// I have to do the WEventSocket::Close() first so that it will EventSelect(0) the m_hSocket.  TSocket::Close() just frees memory.
	if((WEventSocket::Close()==SOCKET_ERROR) || (TSocket::Close()==SOCKET_ERROR))
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
void TEventSocket::Attach(SOCKET hSocket)
{
	WEventSocket::Attach(hSocket);
	TSocket::Attach(hSocket);	// not really necessary, but I just like being thorough :)
}

//
//
//
SOCKET TEventSocket::Detach()
{
	// I have to do the WEventSocket::Detach() first so that it will EventSelect(0) the m_hSocket. TSocket::Detach() just frees memory.
	SOCKET hSocket=WEventSocket::Detach();
	TSocket::Detach();	// will return an INVALID_SOCKET
	return hSocket;
}

//
//
//
int TEventSocket::Listen()
{
	// Redo the event select crap, only waiting for incoming connections
	EventSelect(FD_ACCEPT);
	return listen(m_hSocket,SOMAXCONN);
}

//
//
//
void TEventSocket::OnSend(int error_code)
{
	TSocket::OnSend(error_code);
}