#include "StdAfx.h"
#include "AresDCInterdictionServer.h"
#include "AresDataCollectorSystem.h"

#define ARESINTERDICTIONSERVERPORT 4690
#define ARESINTERDICTIONCLIENTPORT 4691
#define ARESINTERDICTIONSERVERCODE 55
#define ARESINTERDICTIONCLIENTCODE 56
#define ARESINTERDICTIONGETTARGETS 100
#define ARESINTERDICTIONTARGETRESPONSE 101

AresDCInterdictionServer::AresDCInterdictionServer(void)
{
	AfxSocketInit();
	BOOL stat=m_rcv_socket.Create(ARESINTERDICTIONSERVERPORT,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP AntiLeak Socket %d\n",error);
	}
	stat=m_send_socket.Create(0,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP AntiLeak Socket %d\n",error);
	}
}

AresDCInterdictionServer::~AresDCInterdictionServer(void)
{
}

UINT AresDCInterdictionServer::Run(){
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
		if(nread<2 || (buf[0]!=ARESINTERDICTIONSERVERCODE)){
			Sleep(5);
			continue;
		}

		if(buf[1]==ARESINTERDICTIONGETTARGETS){
			RespondTargets(addr);
		}
	}
	return 0;
}

void AresDCInterdictionServer::StopSystem(void)
{
	m_rcv_socket.Close();
	this->StopThread();
}

void AresDCInterdictionServer::StartSystem(void)
{
	this->StartThread();
}

void AresDCInterdictionServer::RespondTargets(const char* ip)
{
	AresDataCollectorSystemReference ref;
	Vector v_tmp_targets;
	ref.System()->GetInterdictionTargetManager()->GetSomeTargets(v_tmp_targets,15);

	int buf_len=2+4+((int)v_tmp_targets.Size()*(4+2+20+4));
	byte *buf=new byte[buf_len];

	buf[0]=ARESINTERDICTIONCLIENTCODE;
	buf[1]=ARESINTERDICTIONTARGETRESPONSE;

	byte *ptr=buf+2;  //skip past first two opcodes

	*(unsigned int*)ptr=(unsigned int)v_tmp_targets.Size();  //set the number of targets we are sending
	ptr+=4;

	//<count><ip><port><ip><port>...
	for(int i=0;i<(int)v_tmp_targets.Size();i++){
		AresInterdictionTarget* it=(AresInterdictionTarget*)v_tmp_targets.Get(i);
		*(unsigned int*)ptr=it->m_ip;  //add ip
		ptr+=4;
		*(unsigned short*)ptr=(unsigned short)it->m_port;  //add port
		ptr+=2;
		memcpy(ptr,it->m_hash,20);  //add the hash
		ptr+=20;
		*(unsigned int*)ptr=it->m_size;  //add the file size
		ptr+=4;
	}


	if(v_tmp_targets.Size()>0){
		//only respond if we actually have some interdiction targets
		int sent=m_send_socket.SendTo(buf,buf_len,ARESINTERDICTIONCLIENTPORT,ip);
		if(sent==SOCKET_ERROR){
			int error=GetLastError();
			TRACE("AresDCInterdictionServer::RespondTargets() Error sending ping response to %s, error was %d.\n",ip,error);
		}
	}
	delete []buf;
}
