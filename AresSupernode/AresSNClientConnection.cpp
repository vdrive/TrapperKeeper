#include "StdAfx.h"
#include "AresSNClientConnection.h"
#include "AresSupernodeSystem.h"
#include "..\AresProtector\AresPacket.h"
#include "..\AresProtector\AresHost.h"
#include "..\AresProtector\AresUtilityClass.h"
#include "..\tkcom\TinySQL.h"
#include "AresUserName.h"
//#include "AresConstants.h"
//#include <sha.h>
//#include "altsha.h"
#include <mmsystem.h>
#include "..\AresDataCollector\ProcessorJob.h"

#include "zlib\zlib.h"

UINT g_id_counter2=0;

AresSNClientConnection::AresSNClientConnection(PeerSocket *ps)
{
	mp_ps_con=ps;
	ps->SetSocketEventListener(this);
	Init(ps->GetIP(),ps->GetPort());

	//put all other init into the Init() function;
}

void AresSNClientConnection::Init(const char *host_ip,unsigned short port){
	//generate 50 common spoof hashes

	mb_clean=false;

	AresSupernodeSystemRef ref;


	m_str_ip=host_ip;
	m_port=port;

	unsigned int ip1,ip2,ip3,ip4;
	sscanf(host_ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	m_remote_ip=(ip1<<24)|(ip2<<16)|(ip3<<8)|(ip4<<0);

	m_status="Connecting...";

	mb_marked_for_death=false;
	m_create_time=CTime::GetCurrentTime();
	m_death_time=CTime::GetCurrentTime();

	m_state=GetTickCount();	
	mb_dead=false;
	m_id=++g_id_counter2;
//	TRACE("AresSNClientConnection::AresSNClientConnection() id=%d\n",m_id);

	m_con_handle=0;
}

AresSNClientConnection::~AresSNClientConnection(void)
{	
	CleanUp();
}


void AresSNClientConnection::Update(void)
{
	if(mb_dead)
		return;

	if(m_last_receive.HasTimedOut(1200) && !mb_marked_for_death){  //10 minutes of waiting then we drop
		m_status="idle";
		Kill();
		return;
	}

	if(m_last_purge.HasTimedOut(30)){
		PurgeEvents();
		m_last_purge.Refresh();
	}

	CTime now=CTime::GetCurrentTime();
	CTimeSpan alive_time=GetAliveTime();
	CTimeSpan dead_time=CTime::GetCurrentTime()-m_death_time;

	if(!mb_marked_for_death && m_create_time<(now-CTimeSpan(4,0,0,0))){  //if its older than 4 days, lets drop it so we can unload old files
		m_status="Died of old age";
		Kill();
		return;
	}


	//if(!mb_marked_for_death && mv_processor_job.Size()>0){
		//ProcessorJob* job=(ProcessorJob*)mv_processor_job.Get(0);
		//if(job->mb_done){
		//	FinishMessage0x62();
		//	mv_processor_job.Remove(0);
		//}
	//}

	//if(!mb_marked_for_death && mb_fully_connected && m_last_ping_sent.HasTimedOut(60)){
		//m_last_ping_sent.Refresh();
		//SendMessage0x67();
	//}
}


void AresSNClientConnection::Kill(void)
{
	mb_marked_for_death=true;
	m_death_time=CTime::GetCurrentTime();
	m_status+=" - Destroying";
	CleanUp();
}

CString AresSNClientConnection::GetFormattedAgeString(void)
{
	CString tmp;
	CTimeSpan dif=CTime::GetCurrentTime()-m_create_time;
	unsigned int total_secs=(unsigned int)dif.GetTotalSeconds();
	static unsigned int secs_in_day=60*60*24;
	static unsigned int secs_in_hour=60*60;
	static unsigned int secs_in_minute=60;
	unsigned int days=total_secs/secs_in_day;
	unsigned int remaining_secs=total_secs-days*secs_in_day;
	unsigned int hours=remaining_secs/secs_in_hour;
	remaining_secs-=(hours*secs_in_hour);
	unsigned int minutes=remaining_secs/secs_in_minute;
	remaining_secs-=(minutes*secs_in_minute);

	tmp.Format("%.2d:%.2d:%.2d:%.2d",days,hours,minutes,remaining_secs);

	return tmp;
}

void AresSNClientConnection::ProcessPacket(AresPacket& packet)
{
	if(mb_marked_for_death || !mp_ps_con)
		return;
	int type=packet.GetMessageType();

	AresSupernodeSystemRef ref;
	CString log_msg;
	if(type!=0x07){
		log_msg.Format("AresSNClientConnection %s:%u ProcessPacket() type=0x%X length=%u",this->m_str_ip.c_str(),this->m_port,type,packet.GetLength());
		TRACE("%s\n",log_msg);
	}
	ref.System()->LogToFile(log_msg);

	
	if(packet.GetMessageType()==0x5d && packet.GetLength()==0x03){
		//we are looking for supernode connections
		//init message
		//respond with a 0x38 message
		//unsigned char data[]={0x03,0x00,0x5a,0x04,0x03,0x05};

	}
	else if(packet.GetMessageType()==0x5a){
		//Kill();  //we don't like clients at this time
	}
	else if(packet.GetMessageType()==0x7){
		Kill();  //we don't like whomever/whatever this is
	}
}

LinkedList* AresSNClientConnection::GetEventList(void)
{
	return &m_event_list;
}


/*
#ifdef TKSOCKETSYSTEM

void AresSNClientConnection::OnClose(){

	TRACE("AresSNClientConnection::OnClose() %s:%u\n",m_str_ip.c_str(),m_port);

	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("Supernode %s:%u OnClose() =(",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogConnectionInfo(log_msg);
	log_msg.Format("AresSNClientConnection OnClose() BEGIN %s:%u",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogToFile(log_msg);


	mb_connected=false;
	if(mb_has_connected){
		m_status="Connection Lost";
	}
	else
		m_status="Could Not Connect";
	Kill();

	log_msg.Format("AresSNClientConnection OnClose() END %s:%u",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogToFile(log_msg);
}

void AresSNClientConnection::OnSend(){
	//TRACE("AresSNClientConnection::OnSend() %s\n",m_str_ip.c_str());
}

void AresSNClientConnection::OnReceive(byte *data,UINT length){
	if(length>0)
		m_last_receive.Refresh();
	AresSupernodeSystemRef ref;

	CString log_msg;
	log_msg.Format("AresSNClientConnection::OnReceive BEGIN %s, %d bytes received",m_str_ip.c_str(),length);
	ref.System()->LogToFile(log_msg);
	

	//TRACE("AresSNClientConnection::OnReceive() %s received %d bytes of data\n",m_str_ip.c_str(),length);
	m_receive_buffer.WriteBytes(data,length);

	AresPacket *packet=new AresPacket();
	if(packet->Read(&m_receive_buffer)){
		Vector v_tmp;
		v_tmp.Add(packet);
		RecordReceiveEvent(packet);
		ProcessPacket(*packet);
	}
	else{ 
		if(packet->IsBad())
			Kill();  //lets close this connection because they sent us something that seems to be invalid
		delete packet;
	};

	log_msg.Format("AresSNClientConnection::OnReceive END %s, %d bytes received",m_str_ip.c_str(),length);
	ref.System()->LogToFile(log_msg);

}

#endif
*/

void AresSNClientConnection::CleanUp(void)
{
	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("AresSNClientConnection::CleanUp() BEGIN %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);


#ifdef TKSOCKETSYSTEM
	if(m_con_handle!=NULL){
		AresSupernodeSystemRef ref;
		ref.System()->GetTCPSystem()->CloseConnection(m_con_handle);
		m_con_handle=NULL;
	}
#else
	if(mp_ps_con){
		if(mp_ps_con->m_hSocket!=INVALID_SOCKET){
			SOCKET s=mp_ps_con->Detach();
			closesocket(s);
		}

		delete mp_ps_con;
	}
	mp_ps_con=NULL;
#endif


	mb_clean=true;
	log_msg.Format("AresSNClientConnection::CleanUp() END %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);

	//_ASSERTE( _CrtCheckMemory( ) );
}

void AresSNClientConnection::RecordReceiveEvent(AresPacket* packet)
{
	CSingleLock lock(&m_event_list.m_list_lock,TRUE);
	AresSupernodeSystemRef ref;
	if(ref.System()->GetEventCacheTime()>0)
		m_event_list.Add(packet);
}

void AresSNClientConnection::RecordSendEvent(AresPacket* packet)
{
	CSingleLock lock(&m_event_list.m_list_lock,TRUE);
	AresSupernodeSystemRef ref;
	if(ref.System()->GetEventCacheTime()>0)
		m_event_list.Add(packet);
}

//call to remove events that have expired past the cache time
void AresSNClientConnection::PurgeEvents(void)
{
	if(m_event_list.Size()==0)
		return;
	m_event_list.StartIteration();
	AresSupernodeSystemRef ref;
	unsigned int cache_time=ref.System()->GetEventCacheTime();
	CTime now=CTime::GetCurrentTime();;
	while(true){
		AresPacket *ap=(AresPacket*)m_event_list.GetCurrent();
		if(!ap)
			break;
		CTimeSpan ts=now-*ap->GetCreatedTime();

		if((ts.GetTotalSeconds()/60)>=cache_time){
			m_event_list.RemoveCurrentAndAdvance();
		}
		else break;  //can just stop here because we know that anything else is going to be even newer
	}
}

void AresSNClientConnection::SendData(byte* data, UINT length)
{
#ifdef TKSOCKETSYSTEM
	AresSupernodeSystemRef ref;
	m_con_handle=ref.System()->GetTCPSystem()->SendData(m_con_handle,data,length);
#else
	mp_ps_con->Send(data,length);
#endif
}

UINT AresSNClientConnection::GetConHandle(void)
{
	return m_con_handle;
}


#ifndef TKSOCKETSYSTEM

void AresSNClientConnection::OnClose(CAsyncSocket* src){
	TRACE("AresSNClientConnection::OnClose() %s:%u\n",m_str_ip.c_str(),m_port);
	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("AresSNClientConnection::OnClose() BEGIN %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);

	log_msg.Format("Supernode %s:%u OnClose() =(",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogConnectionInfo(log_msg);

	m_status="Connection Lost";

	Kill();

	log_msg.Format("AresSNClientConnection::OnClose() END %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);
}


void AresSNClientConnection::OnSend(CAsyncSocket* src){
	//TRACE("AresDCConnection::OnSend() %s\n",m_str_ip.c_str());
}

void AresSNClientConnection::OnReceive(CAsyncSocket* src,byte *data,UINT length){
	if(length>0)
		m_last_receive.Refresh();

	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("AresSNClientConnection::OnReceive() BEGIN %s, %d bytes received",m_str_ip.c_str(),length);
	ref.System()->LogToFile(log_msg);

	//TRACE("AresSNClientConnection::OnReceive() %s received %d bytes of data\n",m_str_ip.c_str(),length);
	m_receive_buffer.WriteBytes(data,length);

	AresPacket *packet=new AresPacket();
	if(packet->Read(&m_receive_buffer)){
		Vector v_tmp;
		v_tmp.Add(packet);
		RecordReceiveEvent(packet);
		ProcessPacket(*packet);
	}
	else{ 
		if(packet->IsBad())
			Kill();  //lets close this connection because they sent us something that seems to be invalid
		delete packet;
	};

	log_msg.Format("AresSNClientConnection::OnReceive() END %s, %d bytes received",m_str_ip.c_str(),length);
	ref.System()->LogToFile(log_msg);

}

#endif

void AresSNClientConnection::SendMessage0x38(void)
{
}
