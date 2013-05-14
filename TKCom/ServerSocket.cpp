// ServerSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ServerSocket.h"
#include "PeerSocket.h"

// ServerSocket

ServerSocket::ServerSocket()
{
}

ServerSocket::~ServerSocket()
{
}


// ServerSocket member functions

void ServerSocket::Init(unsigned short port,SocketEventListener* p_listener)
{
	mp_listener=p_listener;
	m_port=port;
	BOOL stat=-1;
	if(m_bind_ip.size()>0)
		stat=this->Create(port,SOCK_STREAM,FD_ACCEPT,m_bind_ip.c_str());
	else
		stat=this->Create(port,SOCK_STREAM,FD_ACCEPT,NULL);
	int error=-1;
	if(!stat){
		error=GetLastError();
		ASSERT(0);
	}
	stat=this->Listen(5);
	if(!stat){
		error=GetLastError();
		ASSERT(0);
	}
}

void ServerSocket::OnAccept(int nErrorCode)
{
	_ASSERTE( _CrtCheckMemory( ) );
	if(nErrorCode!=0){
		TRACE("ServerSocket::OnAccept() FAILED TO ACCEPT CONNECTION\n");
		CAsyncSocket::OnAccept(nErrorCode);
		_ASSERTE( _CrtCheckMemory( ) );
		return;
	}

	struct sockaddr_in from_addr;
	int address_size=sizeof(sockaddr);

	PeerSocket *nc=new PeerSocket();
	BOOL stat=this->Accept(*nc,(struct sockaddr*)&from_addr,&address_size);
	if(!stat){
		delete nc;
		return;
	}

	nc->AsyncSelect();
	//nc->Set
	char *pcstr_addr=inet_ntoa(from_addr.sin_addr);
	
	//TRACE("ServerSocket::OnAccept() Accepted connection from %s\n",pcstr_addr);
	nc->SetIpPort(pcstr_addr,m_port);
	CString rSockAddr;
	UINT rPort;
	nc->GetSockName(rSockAddr,rPort);
	nc->SetBindIP(rSockAddr);
	mp_listener->OnAccept(this,nc);
	CAsyncSocket::OnAccept(nErrorCode);
	_ASSERTE( _CrtCheckMemory( ) );
}
