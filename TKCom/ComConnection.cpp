#include "StdAfx.h"
#include "comconnection.h"
#include "ComDialog.h"

using namespace comspace;

ComConnection::ComConnection(UINT con_handle,const char* peer)
{
	m_peer=peer;
	m_con_handle=con_handle;
	mb_dirty=false;
	mb_closed=false;
	mb_error=false;
	mb_ready=false;
	mb_sending=false;
}

ComConnection::~ComConnection(void)
{
}

bool ComConnection::NewData(UINT con_handle, byte* data, UINT length)
{
	if(con_handle!=m_con_handle)
		return false;

	if(m_received_data.GetLength()+length>50000000){
		mb_error=true;
		return false;
	}

	m_last_used.Refresh();

	m_received_data.WriteBytes(data,length);
	mb_dirty=true;
	return true;
}

//true indicates it is still ok
//false indicates there has been a serious error and this connection is lost
bool ComConnection::CheckData(Vector &v_received_objects)
{
	if(!mb_dirty)
		return true;
	mb_dirty=false;
	while(true){
		DataBuffer* db=new DataBuffer();
		int stat=db->ReadFromBuffer(m_peer.c_str(),m_received_data.GetMutableBufferPtr(),m_received_data.GetLength());
			
		if(stat<0){
			delete db;
			return false;
		}
		else if(stat==0){
			delete db;
			return true;
		}
		else{
			v_received_objects.Add(db);
			m_received_data.RemoveRange(0,stat);
			g_com_dialog.PostReceive(db->GetOpCode(),m_peer.c_str(),stat);
		}
	}

	//should never get here
	ASSERT(0);
	return false;
}

bool ComConnection::IsPeer(UINT con_id)
{
	if(m_con_handle==con_id)
		return true;
	else return false;
	//if(stricmp(m_con_handle.GetPeer(),ip)==0)
	//	return true;
	//else
	//	return false;
}

bool ComConnection::IsPeer(const char* peer)
{
	if(stricmp(m_peer.c_str(),peer)==0)
		return true;
	else return false;
	//if(stricmp(m_con_handle.GetPeer(),ip)==0)
	//	return true;
	//else
	//	return false;
}


void ComConnection::OnSend(void)
{
	m_last_used.Refresh();
	mb_sending=false;
}

void ComConnection::Closed(void)
{
	mb_closed=true;
}


bool ComConnection::IsExpired(void)
{
	if(m_last_used.HasTimedOut(240))  //if no activity for certain time, kill connection
		return true;
	else return false;
}

void ComConnection::QueueData(DataBuffer* db)
{
	mv_queued_data.Add(db);
}

DataBuffer* ComConnection::GetQueuedData(void)
{
	return (DataBuffer*)mv_queued_data.Get(0);
}

void ComConnection::PopQueuedData(void)
{
	mv_queued_data.Remove(0);
}

void ComConnection::Ready(void)
{
	m_last_used.Refresh();
	mb_ready=true;
}

void ComConnection::StartSending(void)
{
	m_last_used.Refresh();
	mb_sending=true;
}
