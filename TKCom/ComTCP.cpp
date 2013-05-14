#include "StdAfx.h"
#include "comtcp.h"
#include "ComConnection.h"
#include "ComDialog.h"

#define MAXPARALLELCOUNT 3

using namespace comspace;

ComTCP::ComTCP(void)
{
	p_dispatcher=NULL;
}

ComTCP::~ComTCP(void)
{
}

//override this to be notified when data has come in for one of your connections
void ComTCP::OnReceive(UINT con_id , byte* data , UINT length,const char* peer)
{
	//TRACE("ComTCP received data on handle %d of length %d.\n",handle,length);
	int bin_index=GetBinIndex(peer);
	for(UINT i=0;i<mv_connection_bins[bin_index].Size();i++){
		ComConnection *c=(ComConnection*)mv_connection_bins[bin_index].Get(i);
		if(c->NewData(con_id,data,length)){
			return;
		}
	}
}

//override this to be notified when data has been successfully sent to the destination
//if a connection is functioning properly, then you will get 1 OnSend for each send request you made.
void ComTCP::OnSend(UINT con_id,const char* peer)
{
	//TRACE("ComTCP OnSend handle=%d.\n",handle);
	int bin_index=GetBinIndex(peer);
	for(UINT i=0;i<mv_connection_bins[bin_index].Size();i++){
		ComConnection *c=(ComConnection*)mv_connection_bins[bin_index].Get(i);
		if(c->IsConnection(con_id)){
			c->OnSend();
			return;
		}
	}
}

//Override this to be notified when a particular connection has been closed for any number of reasons.  This will not be called if you explicitly called CloseConnection().
void ComTCP::OnClose(UINT con_id,UINT reason, const char* peer)
{
	//TRACE("Com Service:  ComTCP OnClose %d.\n",handle);
	int bin_index=GetBinIndex(peer);
	for(UINT i=0;i<mv_connection_bins[bin_index].Size();i++){
		ComConnection *c=(ComConnection*)mv_connection_bins[bin_index].Get(i);
		if(c->IsConnection(con_id)){
			c->Closed();
			return;
		}
	}
}

//override this to keep new connections from being automatically closed.
//handle is how you reference your new connection.
void ComTCP::OnAcceptedConnection(UINT server_handle,UINT con_id,const char* peer,unsigned short port)
{
	UINT count=0;
	int bin_index=GetBinIndex(peer);
	for(UINT i=0;i<mv_connection_bins[bin_index].Size();i++){
		ComConnection *c=(ComConnection*)mv_connection_bins[bin_index].Get(i);
		if(c->IsPeer(peer)){
			count++;
			if(count>3){ //if too many connections exist, then lets NOT accept this one.  In case of an attack??
				TRACE("Com Service:  NOT Accepting a new connection from %s because too many connections exist already.\n",peer);
				TKSocketSystem::OnAcceptedConnection(server_handle,con_id,peer,port);  //default handling (close it)
				return;
			}
		}
	}

	//TRACE("Com Service:  Accepting a new connection from %s.\n",source_address);
	ComConnection *nc=new ComConnection(con_id,peer);
	nc->Ready();
	mv_connection_bins[bin_index].Add(nc);
}

//notification that a OpenConnection call has completed.
void ComTCP::OnConnect(UINT con_id, const char* peer)
{
	//TRACE("ComTCP::OnConnect() %d.\n",handle);
	int bin_index=GetBinIndex(peer);
	for(UINT i=0;i<mv_connection_bins[bin_index].Size();i++){
		ComConnection *c=(ComConnection*)mv_connection_bins[bin_index].Get(i);
		if(c->IsConnection(con_id)){
			c->Ready();
			return;
		}
	}
}

//lets each connection analyze the data it has received to see if they form any valid objects
void ComTCP::CheckData()
{
	Vector v_received_objects;
	UINT total_connections=0;
	for(int j=0;j<NUMCONBINS;j++){
		for(UINT i=0;i<mv_connection_bins[j].Size();i++){
			ComConnection *c=(ComConnection*)mv_connection_bins[j].Get(i);
			if(!c->CheckData(v_received_objects)){  //is there invalid data buffered? If so, then discard this whole connection.
				mv_connection_bins[j].Remove(i);
				i--;
			}
			else if(c->HasError()){  //if we have an error (like if the peer is trying to send an object that is extremely large)
				//TRACE("ComTCP::CheckData() connection has error peer=%s, handle=%d.\n",c->GetPeer(),c->GetHandle());
				this->CloseConnection(c->GetHandle());
				mv_connection_bins[j].Remove(i);
				i--;
			}
			else if(c->IsClosed()){
				//TRACE("ComTCP::CheckData() connection closed peer=%s, handle=%d.\n",c->GetPeer(),c->GetHandle());
				mv_connection_bins[j].Remove(i);
				i--;
			}
			else if(c->IsExpired()){
				//TRACE("ComTCP::CheckData() connection expired peer=%s, handle=%d.\n",c->GetPeer(),c->GetHandle());
				this->CloseConnection(c->GetHandle());
				mv_connection_bins[j].Remove(i);
				i--;
			}
			else if(c->HasQueuedData() && !c->IsSending() && c->IsReady()){
				//TRACE("ComTCP::CheckData() connection sending data to peer=%s, handle=%d.\n",c->GetPeer(),c->GetHandle());
				DataBuffer* db=c->GetQueuedData();
				g_com_dialog.PostSend(db->GetOpCode(),c->GetPeer(),db->GetDataLength());
				c->StartSending();
				this->SendData(c->GetHandle(),db->GetBuffer(),db->GetDataLength());
				c->PopQueuedData();
			}
		}
		total_connections+=mv_connection_bins[j].Size();
	}

	for(UINT j=0;j<v_received_objects.Size();j++){ //for each data object that it parsed out, route it out to the plugins
		DataBuffer* db=(DataBuffer*)v_received_objects.Get(j);
		p_dispatcher->DispatchMessage(db);  //route our recieved data out to the plugins
	}

	g_com_dialog.SetTotalTCPConnections(total_connections);
}

void ComTCP::SetDispatcher(MessageDispatcher* dispatcher){
	p_dispatcher=dispatcher;
}

bool ComTCP::CanSend(const char* dest, DataBuffer* buffer)
{
	//check to see if any connections are currently open to the destination.
	//TRACE("Com Service: ComTCP::CanSend()  dest=%s.\n",dest);
	UINT count=0;
	int bin_index=GetBinIndex(dest);
	for(int i=((int)mv_connection_bins[bin_index].Size())-1;i>=0;i--){  //iterate through backwards so we can send on the newest connection to a peer
		ComConnection *c=(ComConnection*)mv_connection_bins[bin_index].Get(i);
		if(c->IsPeer(dest)){
			count++;
			if(!c->IsTooBusy()){  //is this connection not being used?
				//TRACE("ComTCP:  Queuing data to %s.\n",dest);
				c->QueueData(buffer);
				return true;
			}
			else if(count>2){
				return false;
			}
			//else if(count>=MAXPARALLELCOUNT){ //return false if too many busy connections exist to this destination.
			//	return false;
			//}
		}
	}

	//room for another connection,thus we will open a new connection. 
	//TKTCPConnection new_con(dest,TKRELIABLEPORT);
	//UINT handle;
	//TRACE("Com Service:  ComTCP::CanSend()  Opening a new connection to %s.\n",dest);
	UINT handle=this->OpenConnection(dest,TKRELIABLEPORT);
	if(!handle)
		return false;
	ComConnection *nc=new ComConnection(handle,dest);
	nc->QueueData(buffer);
	mv_connection_bins[bin_index].Add(nc);
	return true;
}

int ComTCP::GetBinIndex(const char* peer)
{
	return inet_addr(peer)%NUMCONBINS;
}
