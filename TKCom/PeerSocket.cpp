// PeerSocket.cpp : implementation file
//

#include "stdafx.h"
#include "PeerSocket.h"


byte g_receive_buffer[16192];
// PeerSocket

PeerSocket::PeerSocket()
{
	mp_listener=NULL;
	mp_tmp_receive_buffer=NULL;
	m_tmp_receive_buffer_length=0;
}

PeerSocket::~PeerSocket()
{
	if(mp_tmp_receive_buffer){
		delete []mp_tmp_receive_buffer;
		mp_tmp_receive_buffer=NULL;
	}
}


// PeerSocket member functions

void PeerSocket::Init(const char* peer, unsigned short port,SocketEventListener* p_listener)
{
	//_ASSERTE( _CrtCheckMemory( ) );
	mp_listener=p_listener;
	int error=-1;

	BOOL stat=-1;
	if(m_bind_ip.size()>0)
		stat=this->Create(0,SOCK_STREAM,FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE,m_bind_ip.c_str());
	else
		stat=this->Create(0,SOCK_STREAM,FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE,NULL);

//	if(m_bind_ip.size()>0){  //do we need to bind to a specific address?
//		this->Bind(0,m_bind_ip.c_str());
//	}

	if(!stat){
		error=GetLastError();
		TRACE("Ares PeerSocket::Init() Socket creation to %s:%u failed because of %d.\n",peer,port,error);
	}

	stat=this->Connect(peer,port);
	
	if(!stat){
		error=GetLastError();
		if(error!=10035)  //if not equal to "WOULD BLOCK"
			TRACE("Ares PeerSocket::Init() Socket creation to %s:%u failed because of %d.\n",peer,port,error);
	}
	m_ip=peer;
	m_port=port;
	m_time_created=CTime::GetCurrentTime();
	//_ASSERTE( _CrtCheckMemory( ) );
}

void PeerSocket::OnClose(int nErrorCode)
{
	CAsyncSocket::OnClose(nErrorCode);
	//TRACE("PeerSocket::OnClose() %s %d.\n",m_ip.c_str(),m_port);
	//_ASSERTE( _CrtCheckMemory( ) );
	if(mp_listener)
		mp_listener->OnClose(this);
	
	//_ASSERTE( _CrtCheckMemory( ) );
}

void PeerSocket::OnConnect(int nErrorCode)
{
	CAsyncSocket::OnConnect(nErrorCode);
	//TRACE("PeerSocket::OnConnect() %s %d with code %d.\n",m_ip.c_str(),m_port,nErrorCode);
	//_ASSERTE( _CrtCheckMemory( ) );
	if(mp_listener){
		if(nErrorCode==0)
			mp_listener->OnConnect(this);
		else
			mp_listener->OnConnectFailed(this);
	}
	
	//_ASSERTE( _CrtCheckMemory( ) );
}

void PeerSocket::OnReceive(int nErrorCode)
{
	CAsyncSocket::OnReceive(nErrorCode);
	//TRACE("PeerSocket::OnReceive() %s %d.\n",m_ip.c_str(),m_port);
	//_ASSERTE( _CrtCheckMemory( ) );
	//byte buff[4096];
	int nRead;
	nRead = Receive(g_receive_buffer, 16192); 
	   
	switch (nRead)
	{
	case 0:
		Close();
		OnClose(0);  //not sure if this gets called or not
		break;
	case SOCKET_ERROR:
		if (GetLastError() != WSAEWOULDBLOCK) 
		{
			Close();
			OnClose(0);  //not sure if this gets called or not
		}
		break;
	default:
		if(mp_listener){
			mp_listener->OnReceive(this,g_receive_buffer,nRead);
		}
		else{
			//we don't yet have a socket event listener, we want to save this data for when we do
			if(mp_tmp_receive_buffer==NULL){
				mp_tmp_receive_buffer=new byte[nRead];
				memcpy(mp_tmp_receive_buffer,g_receive_buffer,nRead);
				m_tmp_receive_buffer_length=nRead;
			}
			else if(m_tmp_receive_buffer_length<1000000){
				byte *tmp=new byte[m_tmp_receive_buffer_length+nRead];
				memcpy(tmp,mp_tmp_receive_buffer,m_tmp_receive_buffer_length);
				memcpy(tmp+m_tmp_receive_buffer_length,g_receive_buffer,nRead);
				m_tmp_receive_buffer_length+=nRead;
				delete []mp_tmp_receive_buffer;
				mp_tmp_receive_buffer=tmp;
			}
		}
	}
	
	//_ASSERTE( _CrtCheckMemory( ) );
}

void PeerSocket::OnSend(int nErrorCode)
{
	CAsyncSocket::OnSend(nErrorCode);
	//TRACE("PeerSocket::OnSend() %s %d.\n",m_ip.c_str(),m_port);
	//_ASSERTE( _CrtCheckMemory( ) );
	if(nErrorCode==0 && mp_listener)
		mp_listener->OnSend(this);
	
	//_ASSERTE( _CrtCheckMemory( ) );
}

void PeerSocket::SetIpPort(const char* peer, unsigned short port)
{
	m_ip=peer;
	m_port=port;
}

void PeerSocket::SetSocketEventListener(SocketEventListener *p_listener)
{
	mp_listener=p_listener;	
	if(mp_tmp_receive_buffer!=NULL){
		//report the data that we have saved
		mp_listener->OnReceive(this,mp_tmp_receive_buffer,m_tmp_receive_buffer_length);
		delete []mp_tmp_receive_buffer;
		mp_tmp_receive_buffer=NULL;
		m_tmp_receive_buffer_length=0;
	}
}

void PeerSocket::SetBindIP(const char* bind_ip)
{
	m_bind_ip=bind_ip;
}