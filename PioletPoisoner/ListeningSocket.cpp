// ListeningSocket.cpp

#include "stdafx.h"
#include "ListeningSocket.h"
#include "ConnectionModule.h"


ListeningSocket::ListeningSocket()
{
	m_waiting_for_connection = false;
}
//
//
//
void ListeningSocket::InitParent(ConnectionModule *parent)
{
	p_parent=parent;
}

//
//
//
void ListeningSocket::OnAccept(int error_code)
{
	if(error_code==0)
	{
		SOCKET hSocket = Accept();
		p_parent->AcceptedConnection(hSocket,m_port,m_ip,m_filesize,m_md5);
	}
	else
	{
		int err = GetLastError();
		TRACE("Listening socket OnAccept() error: %d\n",err);
	}
	//Close();
	m_waiting_for_connection = false;
	
	//this->m_port = 0;
	//this->m_filesize = 0;
	//this->m_md5 = "";
	//this->m_ip="";
	//p_parent->KillListeningSocket(this);
}

//
//
//
int ListeningSocket::Create(unsigned int port, const char* ip, UINT filesize, const char* md5)
{
	//Close();
	this->m_ip = ip;
	this->m_port = port;
	this->m_filesize = filesize;
	this->m_md5 = md5;
	this->m_creation_time = CTime::GetCurrentTime();
	m_waiting_for_connection = true;
	
	if(!IsSocket())
	{
		int ret = WAsyncSocket::Create(port);
		return Listen();
	}
	return 0;
}

//
//
//
void ListeningSocket::OnClose(int error_code)
{
	Listen();
}

void ListeningSocket::CheckIdle()
{
	if((CTime::GetCurrentTime()-m_creation_time).GetTotalMinutes()>0)
	{
		m_waiting_for_connection=false;
	}
}