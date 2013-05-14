#include "StdAfx.h"
#include "metatcp.h"
#include "buffer2000.h"
#include "metasystem.h"
#include <mmsystem.h>

MetaTCP* MetaTCP::sm_tcp=NULL;
UINT MetaTCP::sm_tcp_count=0;

MetaTCP::MetaTCP(void)
{
}

MetaTCP::~MetaTCP(void)
{
}

void MetaTCP::OnReceive(UINT handle , byte* data , UINT length)
{
	string name;
	unsigned short port;
	this->GetPeerInfo(handle,name,port);
	//TRACE("MetaTCP::OnReceive() from %s on port %d.\n",name.c_str(),port);

	/*
	for(UINT i=0;i<length;i++){
		if(isalnum(data[i])){
			TRACE("MetaTCP::OnReceive() recv[%d] \t = \t 0x%x \t %c \n",i,data[i],data[i]);
		}
		else{//0x%x
			TRACE("MetaTCP::OnReceive() recv[%d] \t = \t 0x%x \n",i,data[i]);
		}
	}*/
	
	MetaSystemReference ref;
	Buffer2000 buffer(data,length);
	ref.System()->NewData(handle,name.c_str(),buffer);
}

void MetaTCP::OnSend(UINT handle)
{
	//string name;
	//unsigned short port;
	//this->GetPeerInfo(handle,name,port);
	//TRACE("MetaTCP::OnSend() to %s on port %d\n",name.c_str(),port);
}

void MetaTCP::OnServerError(UINT handle,UINT error)
{
	TRACE("MetaTCP::OnServerError() error=%d.\n",error);
}

//Override this to be notified when a particular connection has been closed for any number of reasons.  This will not be called if you explicitly called CloseConnection().
void MetaTCP::OnClose(UINT handle,UINT reason)
{
	string name;
	unsigned short port;
	this->GetPeerInfo(handle,name,port);
	//TRACE("MetaTCP::OnClose() peer was %s on port %d, reason for close is %d\n",name.c_str(),port,reason);
	
	MetaSystemReference ref;
	ref.System()->LostConnection(handle,name.c_str());
}

//override this to keep new connections from being automatically closed.
//handle is how you reference your new connection.
void MetaTCP::OnAcceptedConnection(UINT server_handle,UINT connection_handle, const char* source_address)
{
	string name;
	unsigned short port;
	this->GetPeerInfo(connection_handle,name,port);
	TRACE("MetaTCP::OnAcceptedConnection() from %s on port %d.\n",name.c_str(),port);

	MetaSystemReference ref;
	ref.System()->NewConnection(connection_handle,source_address);
}

//notification that a OpenConnection call has completed.
void MetaTCP::OnConnect(UINT handle)
{
	string name;
	unsigned short port;
	this->GetPeerInfo(handle,name,port);
	TRACE("MetaTCP::OnConnect() to %s on port %d.\n",name.c_str(),port);

}
