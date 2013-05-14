#include "StdAfx.h"
#include "listeningsocket.h"
#include "ConnectionManager.h"

ListeningSocket::ListeningSocket(void)
{
}

ListeningSocket::~ListeningSocket(void)
{
}

void ListeningSocket::InitParent(ConnectionManager * parent, int socknum)
{
	p_parent = parent;
	m_socknum = socknum;
}





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
		sockets.push_back(hSocket);
	}
	
	// Do something with the accepted handle, like give it to your parent and they will create a new socket and attach the handle to it.
	p_parent->IncomingConnection(sockets,m_socknum);

}