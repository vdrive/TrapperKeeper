// WSocket.cpp

#include "stdafx.h"
#include "WSocket.h"

//
//
//
WSocket::WSocket()
{
	m_hSocket=INVALID_SOCKET;
	m_socket_type=0;
}

//
//
//
WSocket::~WSocket()
{
	Close();
}

//========================
// Static Member Functions
//========================

//
//
//
int WSocket::Startup()
{
	// Start Windows Sockets
	WSADATA wsdata;
	memset(&wsdata,0,sizeof(WSADATA));
	return WSAStartup(0x0202,&wsdata);
}

//
//
//
void WSocket::Cleanup()
{
	// Cleanup Windows Sockets
	WSACleanup();
}

//
//
//
int WSocket::GetLastError()
{
	return WSAGetLastError();
}

// ===

//
//
//
bool WSocket::IsSocket()
{
	if((m_hSocket==INVALID_SOCKET) || (m_hSocket==SOCKET_ERROR))
	{
		return false;
	}
	else
	{
		return true;
	}
}

// ===

//
// Type is SOCK_STREAM or SOCK_DGRAM
//
int WSocket::Create(unsigned short int port,unsigned int ip/*=INADDR_ANY*/)
{
	int ret=0;

	// Check to see that this is not already a socket
	if(!IsSocket())
	{
		// Create a socket handle that is not overlapped
		if(port==0)
			m_hSocket=WSASocket(AF_INET,m_socket_type,0,NULL,0,0);
		else
			m_hSocket=WSASocket(AF_INET,m_socket_type,0,NULL,0,WSA_FLAG_OVERLAPPED);

		if(m_hSocket==INVALID_SOCKET)
		{
			return INVALID_SOCKET;
		}

		// bind socket to the port they entered in
		sockaddr_in addr;
		ZeroMemory(&addr,sizeof(sockaddr_in));
		addr.sin_family=AF_INET;
//		addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
		addr.sin_addr.S_un.S_addr=ip;
		addr.sin_port=htons(port);

		// Bind the socket, in preparation for listening
		ret=bind(m_hSocket,(sockaddr *)&addr,sizeof(sockaddr_in));
	}

	return ret;
}

//
//
//
int WSocket::Close()
{
	int ret=0;

	// If this is a valid socket
	if(IsSocket())
	{
		// Free the socket handle and reset it
		ret=closesocket(m_hSocket);
		m_hSocket=INVALID_SOCKET;
	}

	return ret;
}

//
//
//
int WSocket::Listen()
{
	return listen(m_hSocket,SOMAXCONN);
}

//
//
//
bool WSocket::Attach(SOCKET hSocket)
{
	// If the socket is not attached, then we can attach this handle to it
	if(m_hSocket==INVALID_SOCKET)
	{
		m_hSocket=hSocket;

		/*
		int ret = WSAEventSelect(m_hSocket,0,0);
		// Make the socket synchrnonous (blocking)
		unsigned long arg_in=0,arg_out,num_out;
		ret=WSAIoctl(m_hSocket,FIONBIO,&arg_in,sizeof(unsigned long),&arg_out,sizeof(unsigned long),
			&num_out,NULL,NULL);

		int error=0;
		if(ret==SOCKET_ERROR)
		{
			error=WSAGetLastError();
		}
		*/

		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
SOCKET WSocket::Detach()
{
	SOCKET hSocket=m_hSocket;
	m_hSocket=INVALID_SOCKET;
	return hSocket;
}

//
//
//
SOCKET WSocket::Accept()
{
	return WSAAccept(m_hSocket,NULL,NULL,NULL,0);
}