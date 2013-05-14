#include "StdAfx.h"
#include ".\Listener.h"

#include "SoulSeekServerDll.h"

Listener::Listener(void)
{
	p_parent=NULL;
}

Listener::~Listener(void)
{
}

//
//
//
void Listener::InitParent(SoulSeekServerDll *parent)
{
	p_parent=parent;
}

//
//
//
void Listener::OnAccept(int error_code)
{
	// Accept the new socket
	sockaddr_in sinRemote;
	int nAddrSize = sizeof( sinRemote );
	ZeroMemory( &sinRemote, nAddrSize );
	SOCKET hSocket = accept( m_hSocket, (sockaddr *)&sinRemote, &nAddrSize );

	// Accept all of the waiting incoming connections
	//SOCKET hSocket=Accept();
	if(hSocket==INVALID_SOCKET)
	{
		return;
	}
	
	char* ip = new char[16];
	strcpy(ip, inet_ntoa(sinRemote.sin_addr));

	// Do something with the accepted handle, like give it to your parent and they will create a new socket and attach the handle to it.
	p_parent->ReceivedIncomingConnections(hSocket,ip);
}