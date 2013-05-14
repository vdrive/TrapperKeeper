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
	vector<SOCKET> sockets;
	while(1)
	{
		SOCKET hSocket=Accept();
		if(hSocket==INVALID_SOCKET)
		{
			break;
		}
		else
			//p_manager->AcceptedConnection(hSocket);
			sockets.push_back(hSocket);
	}
	if(sockets.size()>0)
		p_manager->AcceptedConnection(sockets);

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
