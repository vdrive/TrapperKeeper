#include "StdAfx.h"
#include "supernodeserversysteminterface.h"

#define ARESSUPERNODESERVERPORT 4692
#define ARESSUPERNODECLIENTPORT 4693
#define ARESSUPERNODESERVERCODE 57
#define ARESSUPERNODECLIENTCODE 58
#define ARESSUPERNODEGETSUPERNODES 100
#define ARESSUPERNODEREPORTSUPERNODES 101


SupernodeServerSystemInterface::SupernodeServerSystemInterface(void)
{
	AfxSocketInit();
	BOOL stat=m_rcv_socket.Create(ARESSUPERNODECLIENTPORT,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP SupernodeServerSystemInterface Socket %d\n",error);
	}
	stat=m_send_socket.Create(0,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP SupernodeServerSystemInterface Socket %d\n",error);
	}
}

SupernodeServerSystemInterface::~SupernodeServerSystemInterface(void)
{
}

UINT SupernodeServerSystemInterface::Run(void)
{
	while(!this->b_killThread){
		byte buf[512];
		CString addr;
		UINT port=0;
		int nread=m_rcv_socket.ReceiveFrom(buf,512,addr,port);  //TY DEBUG
		if(nread==SOCKET_ERROR){
			int error=GetLastError();
			Sleep(10);
			continue;
//			TRACE("AresDCInterdictionServer::Run() failed to receive udp data because %d.\n",error);
		}
		if(nread<2 || (buf[0]!=ARESSUPERNODECLIENTCODE)){
			continue;
		}

		byte *p_data=buf+2;

		if(buf[1]==ARESSUPERNODEREPORTSUPERNODES && nread>=4){
			WORD count=*(WORD*)p_data;
			p_data+=2;
			vector<string> v_ips;
			vector<unsigned short> v_ports;
			for(int i=0;i<count && p_data<=(buf+nread-6);i++){
				UINT ip=*(UINT*)p_data;
				p_data+=4;
				unsigned short port=*(unsigned short*)p_data;
				p_data+=2;

				char str_ip[16];
				sprintf( str_ip , "%u.%u.%u.%u" , (ip>>0)&0xff , (ip>>8)&0xff , (ip>>16) & 0xff ,(ip>>24) & 0xff );  //little endian

				v_ips.push_back(str_ip);
				v_ports.push_back(port);
				
			}
			if(v_ips.size()>0)
				ReceivedSupernodes(v_ips,v_ports);  //report this to the derived class
		}
	}
	return 0;
}

void SupernodeServerSystemInterface::StartSystem(void)
{
	this->StartThreadLowPriority();
}

void SupernodeServerSystemInterface::StopSystem(void)
{
	m_rcv_socket.Close();
	m_send_socket.Close();
	this->StopThread();
}

void SupernodeServerSystemInterface::ReceivedSupernodes(vector<string> &v_ips, vector<unsigned short> &v_ports)
{
}

void SupernodeServerSystemInterface::RequestSupernodes(void)
{
	byte buf[2];
	buf[0]=ARESSUPERNODESERVERCODE;
	buf[1]=ARESSUPERNODEGETSUPERNODES;

	int sent=m_send_socket.SendTo(buf,2,ARESSUPERNODESERVERPORT,CONTROLLER_ADDRESS);
	if(sent==SOCKET_ERROR){
		int error=GetLastError();
		TRACE("SupernodeServerSystemInterface::RequestSupernodes() Error sending udp packet to %s, error was %d.\n",CONTROLLER_ADDRESS,error);
	}
}

void SupernodeServerSystemInterface::ReportNewSupernodesToServer(vector<string> &v_ips,vector<unsigned short> &v_ports)
{
	if(v_ips.size()==0)
		return;

	int buf_len=2+2+(int)min(50,(int)v_ips.size())*6;
	byte *buf=new byte[buf_len];
	buf[0]=ARESSUPERNODESERVERCODE;
	buf[1]=ARESSUPERNODEREPORTSUPERNODES;

	byte *ptr=buf+2;  //skip past first two opcodes
	*(WORD*)ptr=(WORD)min(50,(WORD)v_ips.size());
	ptr+=2;

	for(int i=0;i<(int)min(50,v_ips.size());i++){
		*(UINT*)ptr=inet_addr(v_ips[i].c_str());
		ptr+=4;
		*(unsigned short*)ptr=v_ports[i];
		ptr+=2;
	}

	int sent=m_send_socket.SendTo(buf,buf_len,ARESSUPERNODESERVERPORT,CONTROLLER_ADDRESS);
	if(sent==SOCKET_ERROR){
		int error=GetLastError();
		TRACE("SupernodeServerSystemInterface::ReportNewSupernodes() Error sending udp packet to %s, error was %d.\n",CONTROLLER_ADDRESS,error);
	}
	delete []buf;
}
