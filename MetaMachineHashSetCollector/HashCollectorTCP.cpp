#include "StdAfx.h"
#include "MetaSpooferTCP.h"
#include "buffer2000.h"
#include "spoofingsystem.h"
#include <mmsystem.h>

MetaSpooferTCP* MetaSpooferTCP::sm_tcp=NULL;
UINT MetaSpooferTCP::sm_tcp_count=0;

MetaSpooferTCP::MetaSpooferTCP(void)
{
}

MetaSpooferTCP::~MetaSpooferTCP(void)
{
}

void MetaSpooferTCP::OnReceive(UINT handle , byte* data , UINT length)
{
	string name;
	unsigned short port;
	this->GetPeerInfo(handle,name,port);
	//TRACE("MetaSpooferTCP::OnReceive() from %s on port %d.\n",name.c_str(),port);

	/*
	for(UINT i=0;i<length;i++){
		if(isalnum(data[i])){
			TRACE("MetaSpooferTCP::OnReceive() recv[%d] \t = \t 0x%x \t %c \n",i,data[i],data[i]);
		}
		else{//0x%x
			TRACE("MetaSpooferTCP::OnReceive() recv[%d] \t = \t 0x%x \n",i,data[i]);
		}
	}*/
	
	MetaSpooferReference ref;
	Buffer2000 buffer(data,length);
	ref.System()->NewData(handle,name.c_str(),buffer);
}

void MetaSpooferTCP::OnSend(UINT handle)
{
	string name;
	unsigned short port;
	this->GetPeerInfo(handle,name,port);
	//TRACE("MetaSpooferTCP::OnSend() to %s on port %d\n",name.c_str(),port);
}

void MetaSpooferTCP::OnServerError(UINT handle,UINT error)
{
	TRACE("MetaSpooferTCP::OnServerError() error=%d.\n",error);
}

//Override this to be notified when a particular connection has been closed for any number of reasons.  This will not be called if you explicitly called CloseConnection().
void MetaSpooferTCP::OnClose(UINT handle,UINT reason)
{
	string name;
	unsigned short port;
	this->GetPeerInfo(handle,name,port);
	if(reason!=6){
		TRACE("MetaSpooferTCP::OnClose() peer was %s on port %d, reason for close is %d\n",name.c_str(),port,reason);
	}	
	
	MetaSpooferReference ref;
	ref.System()->LostConnection(handle,name.c_str());
}

//override this to keep new connections from being automatically closed.
//handle is how you reference your new connection.
void MetaSpooferTCP::OnAcceptedConnection(UINT server_handle,UINT connection_handle, const char* source_address)
{
	string name;
	unsigned short port;
	this->GetPeerInfo(connection_handle,name,port);

	MetaSpooferReference ref;
	ref.System()->NewConnection(connection_handle,source_address);
}

//notification that a OpenConnection call has completed.
void MetaSpooferTCP::OnConnect(UINT handle)
{
	string name;
	unsigned short port;
	this->GetPeerInfo(handle,name,port);
	TRACE("MetaSpooferTCP::OnConnect() to %s on port %d.\n",name.c_str(),port);

}
