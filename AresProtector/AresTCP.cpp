#include "StdAfx.h"
#include "arestcp.h"
#include "AresProtectionSystem.h"

AresTCP::AresTCP(void)
{
}

AresTCP::~AresTCP(void)
{
}

void AresTCP::OnAcceptedConnection(UINT server_handle,UINT con_id,const char* peer,unsigned short port){
	AresProtectionSystemReference ref;
	ref.System()->OnAccept(server_handle,con_id,peer,port);
}

void AresTCP::OnReceive(UINT con_id , byte* data , UINT length,const char* peer){
	AresProtectionSystemReference ref;
	ref.System()->OnReceive(con_id,data,length,peer);
}

void AresTCP::OnClose(UINT con_id,UINT reason, const char* peer){
	AresProtectionSystemReference ref;
	ref.System()->OnClose(con_id,reason,peer);
}

void AresTCP::OnConnect(UINT con_id, const char* peer){
	AresProtectionSystemReference ref;
	ref.System()->OnConnect(con_id,peer);
}