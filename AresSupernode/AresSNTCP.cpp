#include "StdAfx.h"
#include "aressntcp.h"
#include "AresSupernodeSystem.h"

AresSNTCP::AresSNTCP(void)
{
}

AresSNTCP::~AresSNTCP(void)
{
}

void AresSNTCP::OnAcceptedConnection(UINT server_handle,UINT con_id,const char* peer,unsigned short port){
	TKSocketSystem::OnAcceptedConnection(server_handle,con_id,peer,port);
}

void AresSNTCP::OnReceive(UINT con_id , byte* data , UINT length,const char* peer){
	AresSupernodeSystemRef ref;
	ref.System()->OnReceive(con_id,data,length,peer);
}

void AresSNTCP::OnClose(UINT con_id,UINT reason, const char* peer){
	AresSupernodeSystemRef ref;
	ref.System()->OnClose(con_id,reason,peer);
}

void AresSNTCP::OnConnect(UINT con_id, const char* peer){
	AresSupernodeSystemRef ref;
	ref.System()->OnConnect(con_id,peer);
}
