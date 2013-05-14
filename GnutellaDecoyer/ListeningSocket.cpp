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
void ListeningSocket::OnClose(int error_code)
{
	Listen();
}


	
