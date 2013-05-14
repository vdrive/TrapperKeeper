#include "StdAfx.h"
#include "serversocketbundle.h"


ServerSocketBundle::ServerSocketBundle(void)
{
}

ServerSocketBundle::~ServerSocketBundle(void)
{
}

void ServerSocketBundle::Init(unsigned short port,SocketEventListener* p_listener,vector <string>&v_system_ips)
{
	m_port=port;
	if(v_system_ips.size()==0){
		ServerSocket *ss=new ServerSocket();
		ss->Init(port,p_listener);
		mv_server_sockets.Add(ss);
	}
	else{
		for(int i=0;i<(int)v_system_ips.size();i++){
			ServerSocket *ss=new ServerSocket();
			ss->SetBindIP(v_system_ips[i].c_str());
			ss->Init(port,p_listener);
			mv_server_sockets.Add(ss);			
		}
	}
}

void ServerSocketBundle::Close(void)
{
	for(int i=0;i<(int)mv_server_sockets.Size();i++){
		ServerSocket *ss=(ServerSocket*)mv_server_sockets.Get(i);
		ss->Close();
	}
}
