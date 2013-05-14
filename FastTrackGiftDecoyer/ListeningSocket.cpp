// ListeningSocket.cpp

#include "stdafx.h"
#include "ListeningSocket.h"
#include "NoiseManager.h"

//
//
//
void ListeningSocket::InitParent(NoiseManager *manager)
{
	p_manager=manager;
}

//
//
//
void ListeningSocket::OnAccept(int error_code)
{
	SOCKET hSocket=Accept();
	p_manager->AcceptedConnection(hSocket);
}

//
//
//
// should never get called since this is a listening socket
// if it does, we are overriding the function so the socket won't get closed
void ListeningSocket::OnClose(int error_code)
{
	//WAsyncSocket::OnClose(error_code);
	//p_manager->OnCloseListeningPort(error_code);
	Listen();
}
