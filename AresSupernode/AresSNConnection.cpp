#include "StdAfx.h"
#include "aressnconnection.h"
#include "AresSupernodeSystem.h"
#include "..\AresProtector\AresPacket.h"
#include "..\AresProtector\AresHost.h"
#include "..\AresProtector\AresUtilityClass.h"
#include "..\tkcom\TinySQL.h"
#include "AresUserName.h"
#include "AresSNSpoof.h"
#include <math.h>
//#include "AresConstants.h"
//#include <sha.h>
//#include "altsha.h"
#include <mmsystem.h>
#include "..\AresDataCollector\ProcessorJob.h"

#include "zlib\zlib.h"

UINT g_id_counter=0;

#define MAX_SPOOF_HASH_COUNT 60
byte g_search_hashes[MAX_SPOOF_HASH_COUNT][20];
byte g_decoy_hashes[MAX_SPOOF_HASH_COUNT][20];


AresSNConnection::AresSNConnection(AresHost* host,unsigned short my_port)
{
	m_my_port=my_port;
	Init(host->GetIP(),host->GetPort());

	//put all other init into the Init() function;
}

AresSNConnection::AresSNConnection(const char* host_ip,int port,unsigned short my_port)
{
	m_my_port=my_port;
	Init(host_ip,port);

	//put all other init into the Init() function;
}

void AresSNConnection::Init(const char *host_ip,unsigned short port){
	//generate 50 common spoof hashes
	srand(10);

	/*
	for(int i=0;i<50;i++){
		for(int j=0;j<20;j++){
			m_spoof_hashes[i][j]=rand()%256;
		}
	}
	srand(GetTickCount());  
	for(int i=50;i<NUM_SPOOF_HASHES;i++){  //do 50 completely random hashes
		for(int j=0;j<20;j++){
			m_spoof_hashes[i][j]=rand()%256;
		}
	}

	for(int i=0;i<NUM_SPOOF_HASHES;i++){
		m_num_results[i]=max(1,NUM_SPOOF_HASHES-40-i*2);
	}

	m_total_results=0;  //this will be the total number of packets or user/file combos that we will send back
	for(int i=0;i<NUM_SPOOF_HASHES;i++){
		m_total_results+=m_num_results[i];
	}*/


	mb_fully_connected=false;
	m_num_sn_peers=0;
	m_num_leaf_peers=0;
	mb_handled_compressed=false;

	m_build_number=0;
	ZeroMemory(m_user_name,sizeof(m_user_name));

	mb_clean=false;
	mb_0x38_client=false;

	AresSupernodeSystemRef ref;
	mb_connecting=true;

	mb_first_ping_sent=false;
	mb_name_sent=false;

	m_num_shares=0;

	m_str_ip=host_ip;
	m_port=port;

	unsigned int ip1,ip2,ip3,ip4;
	sscanf(host_ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	m_remote_ip=(ip1<<24)|(ip2<<16)|(ip3<<8)|(ip4<<0);

	m_status="Connecting...";

	mb_has_connected=false;
	mb_connected=false;
	mb_marked_for_death=false;
	m_create_time=CTime::GetCurrentTime();
	m_death_time=CTime::GetCurrentTime();

	m_state=141;//TYDEBUG //GetTickCount();	
	mb_dead=false;
	m_id=++g_id_counter;
//	TRACE("AresSNConnection::AresSNConnection() id=%d\n",m_id);

	m_con_handle=0;
#ifndef TKSOCKETSYSTEM
	mp_ps_con=NULL;
#endif
}

AresSNConnection::~AresSNConnection(void)
{	
	CleanUp();
}


void AresSNConnection::Update(void)
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

	
	if(!mb_has_connected && !mb_marked_for_death){  //if no OnConnect yet
		int secs=(int)alive_time.GetTotalSeconds();
		if(secs>10){
			m_status="Timed Out";
			Kill();
		}
		return;
	}


	if(!mb_marked_for_death && mv_processor_job.Size()>0){
		ProcessorJob* job=(ProcessorJob*)mv_processor_job.Get(0);
		if(job->mb_done){
			FinishMessage0x62();
			mv_processor_job.Remove(0);
		}
	}

	if(!mb_marked_for_death && mb_fully_connected && m_last_ping_sent.HasTimedOut(60)){
		m_last_ping_sent.Refresh();
		SendMessage0x67();
	}

	//ping stuff
	/*
	if(!mb_name_sent && alive_time.GetTotalSeconds()>5){
		SendName();
	}
	else if(mb_name_sent && !mb_first_ping_sent && alive_time.GetTotalSeconds()>25){
		SendPing();
	}
	else if( mb_name_sent && mb_first_ping_sent && m_last_ping_sent.HasTimedOut(55)){
		SendPing();
	}*/
}

void AresSNConnection::Connect(void)
{
	//TRACE("AresSNConnection::Connect()  Trying to connect to %s:%u\n",m_str_ip.c_str(),m_port);

#ifdef TKSOCKETSYSTEM
	AresSupernodeSystemRef ref;
	m_con_handle=ref.System()->GetTCPSystem()->OpenConnection(m_str_ip.c_str(),m_port);
#else
	mp_ps_con=new PeerSocket();
	mp_ps_con->Init(m_str_ip.c_str(),m_port,this);
#endif
}

bool AresSNConnection::IsConnected(void)
{
	return mb_connected;
}

void AresSNConnection::Kill(void)
{
	mb_marked_for_death=true;
	m_death_time=CTime::GetCurrentTime();
	m_status+=" - Destroying";
	CleanUp();
}

CString AresSNConnection::GetFormattedAgeString(void)
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

void AresSNConnection::ProcessPacket(AresPacket& packet)
{
	if(mb_marked_for_death || !mp_ps_con)
		return;
	int type=packet.GetMessageType();

	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("Supernode %s:%u ProcessPacket() type=0x%X length=%u",this->m_str_ip.c_str(),this->m_port,type,packet.GetLength());

//	if(type!=0xaa){
	//	ref.System()->LogConnectionInfo(log_msg);
	//	ref.System()->LogToFile(log_msg);
		//TRACE("%s\n",log_msg);
//	}

	if(packet.GetMessageType()==0x34){
		if(packet.GetLength()>=(0x80+4)){
			byte *eighty=(byte*)(packet.GetEncryptedData()->GetBufferPtr()+4);//&data[4];	// point to the 0x80 of data
			StartMessage0x62(eighty);
		}
		else{
			Kill();
			return;
		}
	}
	else if(packet.GetMessageType()==0x01){
		mb_fully_connected=true;

		// Send message 0x65, which is zlib compressed and seems to not change when sent to different supernodes ... 
		// all it is is my build number with the 0x22 header and the 0x23 footer
		// i'm saying I am build 2964. build 2965 doesn't include the 0x22 header in this message. (FYI)
		// but yes, i've seen in ethereal that once i receive 0x01 I send thsis 0x65 message immediately thereafter
		unsigned char peer0_2[] = {0x17,0x00,0x65,0x78,0x9c,0x63,0x60,0x50,0x62,0x65,0xd0,0x65,0x60,0x98,0xc2,0xcd,0xc0,0xc8,0xa0,0xcc,0x00,0x00,0x08,0x5f,0x01,0x18};
		//SendSocketData(peer0_2,sizeof(peer0_2));

//#ifdef TKSOCKETSYSTEM
//		AresSupernodeSystemRef ref;
//		ref.System()->GetTCPSystem()->SendData(m_con_handle,peer0_2,sizeof(peer0_2));
//#else
		mp_ps_con->Send(peer0_2,sizeof(peer0_2));
//#endif

/*!*			
		// Send ping ... nah ... just respond to their pings so that we'll know what kind of dude they are
		m_first_ping_sent=true;
		m_time_last_ping_sent=CTime::GetCurrentTime();
		SendSNPing();
*!*/
		// *!* ... errrr ... let's just say everyone's compressed ... see what happens ... hmmm ... seemed like the new type didn't talk to me

		// *!* From ethereal, every 60 seconds from now, I send a 0x67 message
		mb_first_ping_sent=true;	// yeah, it ain't been sent, but I want a 0x67 to be sent every 60 seconds from now on
		m_last_ping_sent.Refresh();
		//m_time_last_ping_sent=CTime::GetCurrentTime();
		return;
	}
	else if(packet.GetMessageType()==0x77){	// query
		ReceivedQuery(packet);

		//alter the query id before we foward it
		*(packet.GetEncryptedData()->GetMutableBufferPtr())=rand()%5;
		*(packet.GetEncryptedData()->GetMutableBufferPtr()+1)=rand()%5;
		*(packet.GetEncryptedData()->GetMutableBufferPtr()+2)=rand()%5;
		*(packet.GetEncryptedData()->GetMutableBufferPtr()+3)=rand()%5;

		if((rand()%5)==0)
			ref.System()->ForwardTCPQuery(packet.GetEncryptedData());
		return;
	}
	else if(packet.GetMessageType()==0x65){
		HandleCompressedPacket(packet);
		return;
	}
	else if(packet.GetMessageType()==0x6f){
		/*

		byte *data=packet.GetEncryptedData()->GetMutableBufferPtr();
		UINT dlen=packet.GetEncryptedData()->GetLength();

		TRACE("Received query hit from %s leaf_id=%u,query_id=%u\n",m_str_ip.c_str(),*(WORD*)data,*(WORD*)(data+2));
		TRACE("\n\nMessage 0x6f DECODE:\n");			
		for(int i=0;i<(int)dlen;i++){
			byte b=data[i];

			if(b!=0){
				TRACE("%d 0x%.2X '%c' \n",i,b,b);
			}
			else{
				TRACE("%d 0x%.2X NULL \n",i,b,b);
			}
		}
		TRACE("\n\n");
		*/
	}
	else if(packet.GetMessageType()==0x67){
		if(packet.GetLength()<11){
			Kill();
			return;
		}
		// Even versions 2964 and 2965 send this message

//				m_compressed=false;
		
//				Log0x67((unsigned char *)data,m_hdr.Len());

		// 0x17,0x00,0x67,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x45,0x2f,0xd0,0xfc,0x0c,0x59,0x47,0x81,0x6c,0xb0,0xfd,0x4f
		unsigned short int leaf_count=*((unsigned short int *)packet.GetEncryptedData()->GetMutableBufferPtr());
		unsigned char sn_count=*(packet.GetEncryptedData()->GetMutableBufferPtr()+10);
		char msg[1024];
		sprintf(msg,"0x67 from %s:%u build %u: %u Leaves - %u Supernodes\r\n",m_str_ip.c_str(),m_port,m_build_number,leaf_count,sn_count);
		//TRACE(msg);
		ref.System()->LogConnectionInfo(msg);
		// *!* In ethereal, when I get a 0x67, I send a 0x68
		SendMessage0x68();

		return;
	}
	else if(packet.GetMessageType()==0x91){	// build number?
		if(packet.GetLength()<4){
			Kill();
			return;
		}
		m_build_number=*((unsigned short int *)(packet.GetEncryptedData()->GetMutableBufferPtr()+2));
		return;
	}
}

LinkedList* AresSNConnection::GetEventList(void)
{
	return &m_event_list;
}


/*
#ifdef TKSOCKETSYSTEM

void AresSNConnection::OnClose(){

	TRACE("AresSNConnection::OnClose() %s:%u\n",m_str_ip.c_str(),m_port);

	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("Supernode %s:%u OnClose() =(",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogConnectionInfo(log_msg);
	log_msg.Format("AresSNConnection OnClose() BEGIN %s:%u",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogToFile(log_msg);


	mb_connected=false;
	if(mb_has_connected){
		m_status="Connection Lost";
	}
	else
		m_status="Could Not Connect";
	Kill();

	log_msg.Format("AresSNConnection OnClose() END %s:%u",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogToFile(log_msg);
}

void AresSNConnection::OnSend(){
	//TRACE("AresSNConnection::OnSend() %s\n",m_str_ip.c_str());
}

void AresSNConnection::OnReceive(byte *data,UINT length){
	if(length>0)
		m_last_receive.Refresh();
	AresSupernodeSystemRef ref;

	CString log_msg;
	log_msg.Format("AresSNConnection::OnReceive BEGIN %s, %d bytes received",m_str_ip.c_str(),length);
	ref.System()->LogToFile(log_msg);
	

	//TRACE("AresSNConnection::OnReceive() %s received %d bytes of data\n",m_str_ip.c_str(),length);
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

	log_msg.Format("AresSNConnection::OnReceive END %s, %d bytes received",m_str_ip.c_str(),length);
	ref.System()->LogToFile(log_msg);

}

void AresSNConnection::OnConnect(){
	TRACE("AresSNConnection::OnConnect %s:%u CONNECTION SUCCESSFUL\n",m_str_ip.c_str(),m_port);
	//m_create_time=CTime::GetCurrentTime();
	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("Supernode %s:%u OnConnect()",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogConnectionInfo(log_msg);

	//unsigned char data[]={0x03,0x00,0x5a,0x04,0x03,0x05};  //non supernode communication
	unsigned char data[]={0x03,0x00,0x5d,0x03,0x04,0x05};  //supernode to supernode communication
	//if(mp_ps_con)
	//	mp_ps_con->Send(data,sizeof(data));
	ref.System()->GetTCPSystem()->SendData(m_con_handle,data,sizeof(data));
	//ref.System()->GetTCP()->SendData(m_con,data,sizeof(data)); //send the initialization data
	mb_has_connected=true;
	mb_connected=true;
	m_status="Connected - sent init packet";

	mb_connecting=false;
}

void AresSNConnection::OnConnectFailed(){
//	TRACE("AresSNConnection::OnConnectFailed() %s\n",m_str_ip.c_str());
	mb_connected=false;
	m_status="Could Not Connect";
	Kill();
}

#endif
*/

void AresSNConnection::CleanUp(void)
{
	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("AresSNConnection::CleanUp() BEGIN %s",m_str_ip.c_str());
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

	mb_connecting=false;

	mb_clean=true;
	log_msg.Format("AresSNConnection::CleanUp() END %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);

	//_ASSERTE( _CrtCheckMemory( ) );
}

void AresSNConnection::RecordReceiveEvent(AresPacket* packet)
{
	CSingleLock lock(&m_event_list.m_list_lock,TRUE);
	AresSupernodeSystemRef ref;
	if(ref.System()->GetEventCacheTime()>0)
		m_event_list.Add(packet);
}

void AresSNConnection::RecordSendEvent(AresPacket* packet)
{
	CSingleLock lock(&m_event_list.m_list_lock,TRUE);
	AresSupernodeSystemRef ref;
	if(ref.System()->GetEventCacheTime()>0)
		m_event_list.Add(packet);
}

//call to remove events that have expired past the cache time
void AresSNConnection::PurgeEvents(void)
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

void AresSNConnection::SendData(byte* data, UINT length)
{
#ifdef TKSOCKETSYSTEM
	AresSupernodeSystemRef ref;
	m_con_handle=ref.System()->GetTCPSystem()->SendData(m_con_handle,data,length);
#else
	mp_ps_con->Send(data,length);
#endif
}

UINT AresSNConnection::GetConHandle(void)
{
	return m_con_handle;
}

bool AresSNConnection::IsFullyConnected(void)
{
	return mb_fully_connected;
}

void AresSNConnection::StartMessage0x62(byte* eighty)
{
	ProcessorJob *new_job=new ProcessorJob(eighty);
	mv_processor_job.Add(new_job);
	AresSupernodeSystemRef ref;
	ref.System()->AddProcessorJob(new_job);  //let the system begin crunching on this one
}

void AresSNConnection::FinishMessage0x62(void)
{
	unsigned char sha[20];
	memset(sha,0,sizeof(sha));

	ProcessorJob* job=(ProcessorJob*)mv_processor_job.Get(0);

	// Bork the data and take the sha-1 of it to get the unencrypted 20 for message 0x62
	memcpy(sha,job->m_twenty_out,20);

	// Encrypt the sha for msg 0x62
	unsigned short int code=0x4224;
	for(int i=0;i<sizeof(sha);i++)
	{	
		unsigned char c=sha[i];
		sha[i]^=(code>>8);

		code+=sha[i];
		code*=0x310f;
		code+=0x3a4e;
	}

	Buffer2000 *response=new Buffer2000();
	response->WriteBytes(sha,20);
	//this is the code the real client uses to find a random port

	//find the initial value of state that will yield our target port of 4685 
	//actually it turns out no value of m_state can yield a target port of 4685
	/*
	for(UINT tmp_index=0;tmp_index<4000000000;tmp_index++){
		if((tmp_index%400000)==0){
			TRACE("reach computation index %u\n",tmp_index);
		}
		UINT tmp=tmp_index;
		unsigned short result=AresUtilityClass::CalcStateShort(0xea60,tmp)+0x1392+1;

		//TRACE("result=%u\n",result);
		if(result>=30400 && result<30500){
			TRACE("FOUND computation index %u for result=%u\n",tmp_index,result);
			break;
		}
	}*/


	unsigned short future_port=AresUtilityClass::CalcStateShort(0xea60,m_state)+0x1392;
	response->WriteWord(future_port);  //tell this supernode we are listening on this port
	response->WriteWord(0);
	response->WriteWord(0);
	response->WriteWord(AresUtilityClass::CalcStateShort(0xfffe,m_state)+1);
	//response->WriteWord(4685);
	response->WriteByte(AresUtilityClass::CalcStateByte(0xfe,m_state)+1);

	// The sha-1 is now encrypted for message 0x62
	//unsigned char data[sizeof(AresHeader)+0x9d];
	//memset(data,0,sizeof(data));
	//unsigned char *ptr=data;

	// Init the header
	//AresHeader *hdr=(AresHeader *)ptr;
	//hdr->Len(sizeof(data)-sizeof(AresHeader));
	//hdr->Op(0x62);	// reply to 0x34
	//ptr+=sizeof(AresHeader);

	// Copy the encrypted sha
	//memcpy(ptr,sha,sizeof(sha));
	//ptr+=sizeof(sha);

	// Calculate and copy the random listening port
	//unsigned short int rand_port=AresUtilityClass::CalcStateShort(0xea60,m_state)+0x1392;
	//memcpy(ptr,&rand_port,sizeof(unsigned short int));
	//ptr+=sizeof(unsigned short int);

	// Increment unknown short of 0
	//ptr+=sizeof(unsigned short int);

	// Increment past what I think is a short of how many other supernodes I am already connected to. I will leave this at 0.
	//ptr+=sizeof(unsigned short int);	

	// Calculate and copy the random short
	//unsigned short int rand_short=CalcStateShort(0xfffe)+1;
	//memcpy(ptr,&rand_short,sizeof(unsigned short int));
	//ptr+=sizeof(unsigned short int);

	// Calculate and copy the random byte
	//unsigned char rand_byte=CalcStateByte(0xfe)+1;
	//*ptr++=rand_byte;

	// Create and copy 8 16-byte GUIDs to the end of the data
	for(int i=0;i<8;i++)
	{
		byte guid[sizeof(GUID)];
		//byte guid[sizeof(GUID)];
		CoCreateGuid((GUID*)&guid);  //give us a random 16 byte array basically, WARNING though this may be useable to track our machines not sure since i believe this incorporates MAC address
		response->WriteBytes(guid,sizeof(GUID));
		//ptr+=sizeof(GUID);
	}

	// Send the data
//	SendSocketData(data,sizeof(data));
	AresPacket *np=new AresPacket();
	Vector v_tmp;
	v_tmp.Add(np);
	
	np->SetMessageType(0x62);
	np->SetEncryptedData(response);
	AresSupernodeSystemRef ref;
	RecordSendEvent(np);

#ifdef TKSOCKETSYSTEM
	np->SendData(m_con_handle,ref.System()->GetTCPSystem());
#else
	np->SendData(mp_ps_con);
#endif
	//ref.System()->GetTCPSystem()->SendData(m_con_handle,response->GetMutableBufferPtr(),response->GetLength());

}

void AresSNConnection::ReceivedQuery(AresPacket& packet)
{
	byte *data=packet.GetEncryptedData()->GetMutableBufferPtr();
	int packet_length=packet.GetEncryptedData()->GetLength();
	byte ttl=data[5];	// i think this is what this is

	unsigned short query_id=*(unsigned short*)data;
	unsigned short leaf_id=*(unsigned short*)(data+2);

	char msg[1024];
	memset(msg,0,sizeof(msg));
//	sprintf(msg,"0x%02x Build %u Query 0x%x Leaf 0x%x - %s:%u (TTL : %i) :",op,m_remote_build,query_id,leaf_id,IP2Str(m_remote_ip).c_str(),m_remote_port,ttl);
	//sprintf(msg,"Query 0x%02x Build %u - %s:%u (TTL : %i) :",op,m_remote_build,IP2Str(m_remote_ip).c_str(),m_remote_port,ttl);

	// Extract keywords
	unsigned char *ptr=(unsigned char *)data;
	ptr+=6;	// point at first keyword

	char keyword[256];
	vector<string> v_keywords;
	while(ptr < ((unsigned char *)(data+min(packet_length,1000)-2)))
	{
		strcat(msg," ");
		memset(keyword,0,256);

		// Extract keyword and append it
		ptr++;						// keyword type
		unsigned char len=*ptr++;	// keyword len
		ptr+=2;						// calculate 2 bytes from buffer
		if(len<256 && ((ptr+len)<=(data+min(packet_length,1000)))){
			memcpy(keyword,ptr,len);
			v_keywords.push_back(keyword);
			memcpy(&msg[strlen(msg)],ptr,len);
		}
		else
			break;  //seems to crash occasionally without this check, either corruption or a slightly different format than we are expecting?
		ptr+=len;
	}

	CTimeSpan ts=CTime::GetCurrentTime()-this->m_create_time;

	UINT age=(UINT)ts.GetTotalSeconds();

	CString log_msg;
	log_msg.Format("Received TCP/IP Query from %s:%u age=%u build=%u ttl=%u search = %s",m_str_ip.c_str(),m_port,age,m_build_number,ttl,msg);

	AresSupernodeSystemRef ref;
	ref.System()->LogQuery(log_msg);

	//TTL table
	//255 = everything
	//1=audio
	//3=video
	//5=image
	//4=document
	//2=software
	//0=other

	if(ttl!=255 && ttl!=1 && ttl!=3)  //only allow all,audio, and video searches for now
		return;


	//TYDEBUG
	TestQuery(ttl,query_id,leaf_id,msg,v_keywords);	


	
	// If special query
//	if((strstr(msg,"eminem")!=NULL && strstr(msg,"just")!=NULL) || strstr(msg,"ubertestfile")|| strstr(msg,"ebertsux") || (strstr(msg,"mariah")!=NULL && strstr(msg,"endless")!=NULL))
//	{
	//	int x=5;
//		RespondToTestQuery(ttl,query_id,leaf_id,msg);
	
//DEL		TinySQL sql;
//DEL		if(!sql.Init("38.119.64.101","onsystems","ebertsux37","ares_data",3306))
//DEL			return;
//DEL
//DEL		CString query;
//DEL		if(mb_handled_compressed)
//DEL			query.Format("insert into query_log values ('%s',%u,'%s','%s',1,now())",msg,m_build_number,m_str_ip.c_str(),ref.System()->GetSystemIP());
//DEL		else
//DEL			query.Format("insert into query_log values ('%s',%u,'%s','%s',0,now())",msg,m_build_number,m_str_ip.c_str(),ref.System()->GetSystemIP());
//DEL
//DEL		sql.Query(query,false);
//	}
}

void AresSNConnection::SendMessage0x68(void)
{
	if(!mp_ps_con)
		return;
	InitPeerCounts();

	unsigned char peer0_3[] = {0x03,0x00,0x68,0x00,0x00,0x27};

	// Reset # of leaf peers connected to me (i think) ... i've seen this number as high as 351
	*((unsigned short int *)&peer0_3[3])=m_num_leaf_peers;	
	
	// Reset # of other supernodes we are connected to (i think) ... it looks like the max is 70
	peer0_3[5]=m_num_sn_peers;		

#ifdef TKSOCKETSYSTEM
	AresSupernodeSystemRef ref;
	ref.System()->GetTCPSystem()->SendData(m_con_handle,peer0_3,sizeof(peer0_3));
#else
	mp_ps_con->Send(peer0_3,sizeof(peer0_3));
#endif
}

void AresSNConnection::InitPeerCounts(void)
{
	// If the num sn peers is 0, initialize it to some random number between 1 and 70
	if(m_num_sn_peers == 0)
	{
		m_num_sn_peers=55+(rand()%5);
	}
	else if(m_num_sn_peers < 70)	// else if it is < 70, see if we can bump it up to get to 70
	{
		// 1 out of 2
		if((rand()&1) == 0)
		{
			m_num_sn_peers++;
		}
	}

	// If the num leaf peers is 0, initialize it to some random number between 1 and 400 (I've seen this value up to 406)
	if(m_num_leaf_peers == 0)
	{
		m_num_leaf_peers=150+(rand()%5);
	}
	else if(m_num_leaf_peers < 350)	// else if it is < X, see if we can bump it up to get to X
	{
		// 1 out of 2
		if((rand()&1) == 0)
		{
			m_num_leaf_peers++;
		}
	}
}

void AresSNConnection::SendMessage0x67(void)
{
	if(!mp_ps_con)
		return;
	InitPeerCounts();

	unsigned char peer0_4[] = {0x17,0x00,0x67,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x45,0x2f,0xd0,0xfc,0x0c,0x59,0x47,0x81,0x6c,0xb0,0xfd,0x4f};

	// Reset # of leaf peers connected to me (i think) ... i've seen this number as high as 351
	*((unsigned short int *)&peer0_4[3])=m_num_leaf_peers;	
	
	// Reset # of other supernodes we are connected to (i think) ... it looks like the max is 70
	peer0_4[13]=m_num_sn_peers;
	
#ifdef TKSOCKETSYSTEM
	AresSupernodeSystemRef ref;
	ref.System()->GetTCPSystem()->SendData(m_con_handle,peer0_4,sizeof(peer0_4));
#else
	mp_ps_con->Send(peer0_4,sizeof(peer0_4));
#endif
}

void AresSNConnection::HandleCompressedPacket(AresPacket& packet)
{
//				m_compressed=true;

	// Decompress it and look at the embedded messages		
	
	UINT original_size=10000;  //a guess
	byte *decomp_data=new byte[original_size];
	UINT len=original_size;
	uncompress ((Bytef*)decomp_data, (uLongf *)&len, (const Bytef *)(packet.GetEncryptedData()->GetBufferPtr()), (uLong)packet.GetEncryptedData()->GetLength());

	Buffer2000 input(decomp_data,len); //make a managed copy and then delete raw byte buffer
	delete []decomp_data;

	if(len==0 || len>=10000){
		Kill();  //evil supernode
		return;
	}

	AresSupernodeSystemRef ref;

	while(input.GetLength()>0){
		AresPacket sub_packet;
		if(!sub_packet.Read(&input)){
			//must be a corrupt packet?
			ASSERT(0);
			Kill();
			return;
		}

		byte type=sub_packet.GetMessageType();

		CString log_msg;
		log_msg.Format("Supernode %s:%u HandleCompressedPacket() type=0x%X length=%u",this->m_str_ip.c_str(),this->m_port,type,sub_packet.GetLength());
		//if(type!=0xaa){
			//ref.System()->LogConnectionInfo(log_msg);
			ref.System()->LogToFile(log_msg);
			//TRACE("%s\n",log_msg);
		//}

		if(type==0x13){
			ReceivedQuery(sub_packet);
		}
		else if(type==0x2d){
			if(sub_packet.GetLength()<4){
				Kill();
				return;
			}
			mb_handled_compressed=true;
			m_build_number=*((unsigned short int *)(sub_packet.GetEncryptedData()->GetMutableBufferPtr()+2));
			SendMessage0x03();  //respond with 0x03 message
		}
	}
}

void AresSNConnection::SendMessage0x03(void)
{
	InitPeerCounts();

	// This is basically message 0x67 with a compressed footer (0x23) at the end

	// *!* did send a 0x13 len 0x03 message, but I've seen it, it's supposed to be 0x17
	unsigned char peer0_4[] = {0x17,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2e,0x44,0xcd,0xc3,0x1f,0xf4,0x1b,0x47,0x65,0x46,0xbe,0xa9,0xcd,0x01,0x00,0x23,0x00};

	// Reset # of leaf peers connected to me (i think) ... i've seen this number as high as 351
	*((unsigned short int *)&peer0_4[3])=m_num_leaf_peers;	
	
	// Reset # of other supernodes we are connected to (i think) ... it looks like the max is 70
	peer0_4[13]=m_num_sn_peers;
	

	byte comp[1024];
	UINT comp_len=1024-3;
	memset(comp,0,sizeof(comp));

	//compress peer0_4 into comp[]
	compress ((Bytef *)&comp, (uLongf *)&comp_len, (const Bytef *)peer0_4, (uLong) sizeof(peer0_4));

	AresPacket *response=new AresPacket();
	response->SetMessageType(0x65);
	response->SetEncryptedData(new Buffer2000(comp,comp_len));
	Vector v_tmp;
	v_tmp.Add(response);
	RecordSendEvent(response);

	AresSupernodeSystemRef ref;

#ifdef TKSOCKETSYSTEM
	response->SendData(m_con_handle,ref.System()->GetTCPSystem());
#else
	response->SendData(mp_ps_con);
#endif


	// Compress the data
//	unsigned char comp[1024];
//	memset(comp,0,sizeof(comp));
//	unsigned int comp_len=sizeof(comp)-sizeof(AresHeader);
//	CompressData(peer0_4,sizeof(peer0_4),&comp[sizeof(AresHeader)],&comp_len);
//	AresHeader *hdr=(AresHeader *)comp;
//	hdr->Op(0x65);
//	hdr->Len(comp_len);

//	SendSocketData(comp,sizeof(AresHeader)+hdr->Len());
}


#ifndef TKSOCKETSYSTEM

void AresSNConnection::OnClose(CAsyncSocket* src){
	//TRACE("AresSNConnection::OnClose() %s:%u\n",m_str_ip.c_str(),m_port);
	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("AresSNConnection::OnClose() BEGIN %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);

	log_msg.Format("Supernode %s:%u OnClose() =(",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogConnectionInfo(log_msg);


	mb_connected=false;
	if(mb_has_connected){
		m_status="Connection Lost";
	}
	else
		m_status="Could Not Connect";
	Kill();

	CTimeSpan tmp_span=GetAliveTime();

	if(mb_fully_connected && tmp_span.GetTotalSeconds()>60*20){  //if we were connected for any significant amount of time, lets try to reestablish a connection to this host
		ref.System()->GetHostCache()->AttemptReconnection(m_str_ip.c_str(),m_port);
	}

	log_msg.Format("AresSNConnection::OnClose() END %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);
}


void AresSNConnection::OnSend(CAsyncSocket* src){
	//TRACE("AresDCConnection::OnSend() %s\n",m_str_ip.c_str());
}

void AresSNConnection::OnReceive(CAsyncSocket* src,byte *data,UINT length){
	if(length>0)
		m_last_receive.Refresh();


	//purge the very frequent 0xaa packets in an efficient manner
	byte *ptr=data;
	if(length>=3 && m_receive_buffer.GetLength()==0){
		int count=0;

		while(length>=3 && (ptr[2]==0xaa || ptr[2]==0xab || ptr[2]==0x6f)){
			WORD plen=*(WORD*)ptr;
			if(plen<=(length-3)){
				ptr+=(plen+3);
				length-=(plen+3);
			}
			else break;
			count++;
		}
		//if(count!=0)
		//	TRACE("%s Efficiently purged %d garbage packets, remaining length=%u\n",m_str_ip.c_str(),count,length);
	}
	if(length==0)
		return;



	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("AresSNConnection::OnReceive() BEGIN %s, %d bytes received",m_str_ip.c_str(),length);
	ref.System()->LogToFile(log_msg);

	//TRACE("AresSNConnection::OnReceive() %s received %d bytes of data\n",m_str_ip.c_str(),length);

	

	m_receive_buffer.WriteBytes(ptr,length);

	bool reading=true;
	while(reading){
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

			reading=false;
			delete packet;
		};
	}

	log_msg.Format("AresSNConnection::OnReceive() END %s, %d bytes received",m_str_ip.c_str(),length);
	ref.System()->LogToFile(log_msg);

}

void AresSNConnection::OnConnect(CAsyncSocket* src){
	if(!mp_ps_con)
		return;

	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("AresSNConnection::OnConnect() BEGIN %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);

	//TRACE("AresSNConnection::OnConnect %s:%u CONNECTION SUCCESSFUL\n",m_str_ip.c_str(),m_port);
	//m_create_time=CTime::GetCurrentTime();
	
	log_msg.Format("Supernode %s:%u OnConnect()",this->m_str_ip.c_str(),this->m_port);
	ref.System()->LogConnectionInfo(log_msg);

	//unsigned char data[]={0x03,0x00,0x5a,0x04,0x03,0x05};  //non supernode communication
	unsigned char data[]={0x03,0x00,0x5d,0x03,0x04,0x05};  //supernode to supernode communication


	//if(mp_ps_con)
	//	mp_ps_con->Send(data,sizeof(data));
#ifdef TKSOCKETSYSTEM
	ref.System()->GetTCPSystem()->SendData(m_con_handle,data,sizeof(data));
#else
	if(mp_ps_con)
		mp_ps_con->Send(data,sizeof(data));
#endif

	mb_has_connected=true;
	mb_connected=true;
	m_status="Connected - sent init packet";

	mb_connecting=false;

	log_msg.Format("AresSNConnection::OnConnect() END %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

void AresSNConnection::OnConnectFailed(CAsyncSocket* src){
	AresSupernodeSystemRef ref;
	CString log_msg;
	log_msg.Format("AresSNConnection::OnConnectFailed() BEGIN %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);

//	TRACE("AresSNConnection::OnConnectFailed() %s\n",m_str_ip.c_str());
	mb_connected=false;
	m_status="Could Not Connect";
	Kill();

	log_msg.Format("AresSNConnection::OnConnectFailed() END %s",m_str_ip.c_str());
	ref.System()->LogToFile(log_msg);
}

#endif


//this is a test function
void AresSNConnection::RespondToTestQuery(int search_type,unsigned short query_id, unsigned short leaf_id, const char* query_str)
{
	//op_code = 0x6f

	AresSupernodeSystemRef ref;
	CString file_name_audio;
	CString file_name_video;



	for(int send_tmp=0;send_tmp<3;send_tmp++){

//		if(send_attempt==0){
			file_name_audio.Format("ubertestfile ebertsux eberttcp%d sn %s srv %s.mp3",send_tmp,m_str_ip.c_str(),ref.System()->GetSystemIP());
			file_name_video.Format("ubertestfile ebertsux eberttcp%d sn %s srv %s.avi",send_tmp,m_str_ip.c_str(),ref.System()->GetSystemIP());
//		}
//		else if(send_attempt==1){
//			file_name_audio.Format("ubertestfile ebertsux ebertudp sn %s srv %s.mp3",m_str_ip.c_str(),ref.System()->GetSystemIP());
//			file_name_video.Format("ubertestfile ebertsux ebertudp sn %s srv %s.avi",m_str_ip.c_str(),ref.System()->GetSystemIP());
//		}
//		else if(send_attempt==2){
//			file_name_audio.Format("ubertestfile ebertsux ebertudp2 sn %s srv %s.mp3",m_str_ip.c_str(),ref.System()->GetSystemIP());
//			file_name_video.Format("ubertestfile ebertsux ebertudp2 sn %s srv %s.avi",m_str_ip.c_str(),ref.System()->GetSystemIP());
//		}

		int num_hits=5+(int)pow(10,send_tmp);

		//load up X user names to spoof
		Vector v_users;
		ref.System()->GetHostCache()->GetSomeUserNames(num_hits,v_users);

		int name_index=0;

		Buffer2000 *packet=new Buffer2000(35000);

		bool b_video=false;
		if(search_type==3)
			b_video=true;

		for(int hash_index=0;hash_index<num_hits && v_users.Size()>0;hash_index++){
			AresUserName *user_name=(AresUserName*)v_users.Get(((name_index++)%v_users.Size()));

			//Buffer2000 *packet=new Buffer2000();
			UINT len_offset=packet->GetLength();
//			if(send_attempt==0){
				packet->WriteWord(0); //room for length;
				packet->WriteByte(0x6f);  //op_code= tcp query hit
				packet->WriteWord(leaf_id);
				packet->WriteWord(query_id);
//			}
//			else if(send_attempt==1){
//				packet->WriteByte('A');
//				packet->WriteByte('R');
//				packet->WriteByte('E');
//				packet->WriteByte(0x1b);  //udp query hit
//				packet->WriteWord(leaf_id);  //opposite of tcp version
//				packet->WriteWord(query_id);
//			}
//			else if(send_attempt==2){
//				packet->WriteByte('A');
//				packet->WriteByte('R');
//				packet->WriteByte('E');
//				packet->WriteByte(0x1b);  //udp query hit
//				packet->WriteWord(query_id);  //opposite of tcp version
//				packet->WriteWord(leaf_id);
//			}
			packet->WriteWord(rand()); //write ip
			packet->WriteWord(rand()); //write ip
			packet->WriteWord(5000+(rand()%10000)); //write port
			packet->WriteByte(0x61);  //wierd code
			packet->WriteString(user_name->m_name.c_str());  //USER NAME
			packet->WriteByte(0);  //NULL
//			if(send_attempt==0){
				packet->WriteByte(0x2a);  //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0x00);     //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0x03);  //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0x04);  //FIGURE OUT WHAT THIS IS
//			}
//			else{
//				packet->WriteByte(0x10);  //FIGURE OUT WHAT THIS IS
//				packet->WriteByte(0);     //FIGURE OUT WHAT THIS IS
//				packet->WriteByte(0);     //FIGURE OUT WHAT THIS IS
//				packet->WriteByte(0x04);  //FIGURE OUT WHAT THIS IS
//			}
			packet->WriteByte(0);  //NULL
			if(b_video)
				packet->WriteByte(0x05);  //type audio
			else
				packet->WriteByte(0x01);  //type audio
			packet->WriteDWord(7654321);  //size

			//write in sha1
			for(int k=0;k<20;k++)
				packet->WriteByte(rand()&255);
			//packet->WriteBytes(m_spoof_hashes[hash_index],20);

			if(b_video){
				packet->WriteByte('.');  //extension
				packet->WriteByte('a');  //extension
				packet->WriteByte('v');  //extension
				packet->WriteByte('i');  //extension
			}
			else{
				packet->WriteByte('.');  //extension
				packet->WriteByte('m');  //extension
				packet->WriteByte('p');  //extension
				packet->WriteByte('3');  //extension
			}
			packet->WriteByte(0);    //NULL

			//title
			packet->WriteByte(0x01);
			packet->WriteString("Ubertestfile ebertsux");
			packet->WriteByte(0);


			//artist
			packet->WriteByte(0x02);
//			if(send_attempt==0)
				packet->WriteString("Ubertestfile ebertsux tcpartist");
//			else
//				packet->WriteString("Ubertestfile ebertsux udpartist");
			packet->WriteByte(0);

			//album
			if(b_video){
				packet->WriteByte(0x03);
//				if(send_attempt==0)
					packet->WriteString("my tcpcategory");
//				else
//					packet->WriteString("my udpcategory");
				packet->WriteByte(0);
			}
			else{
				packet->WriteByte(0x03);
//				if(send_attempt==0)
					packet->WriteString("Ubertestfile ebertsux tcpalbum");
//				else
//					packet->WriteString("Ubertestfile ebertsux udpalbum");
				packet->WriteByte(0);
			}

			//bitrate,medialength
			if(b_video){
				packet->WriteByte(0x04);
				packet->WriteWord(100);
				packet->WriteWord(200);
				packet->WriteWord(1200);
				packet->WriteByte(0);
				packet->WriteByte(0);
			}
			else{
				packet->WriteByte(0x04);
				packet->WriteWord(192);
				packet->WriteDWord(777);
			}

			if(b_video){
			//language
				//video
				packet->WriteByte(0x04);
				packet->WriteString("english");
				packet->WriteByte(0);
			}

			//filename
			packet->WriteByte(0x10);
			if(b_video){
				packet->WriteString((LPCSTR)file_name_video);
			}
			else{
				packet->WriteString((LPCSTR)file_name_audio);
			}
			packet->WriteByte(0x00);

//			if(send_attempt==0){
				packet->WriteWord(len_offset,packet->GetLength()-len_offset-3);
//			}
//			else{
//				AresSNUDPHost tmp_host(m_str_ip.c_str(),m_port);
//				ref.System()->GetUDPDispatcher()->AddPacket(&tmp_host,packet);
//				packet=new Buffer2000(35000);
//			}

			//send this packet
			//AresPacket *tcp_packet=new AresPacket();
			//tcp_packet->ref();
			//tcp_packet->SetType(0x6f);
			//tcp_packet->SetEncryptedData(packet);

			//RecordSendEvent(tcp_packet);
			//tcp_packet->SendData(mp_ps_con);
			//tcp_packet->deref();
		}


//		if(send_attempt==0){
			if(packet->GetLength()>0){
				mp_ps_con->Send(packet->GetMutableBufferPtr(),packet->GetLength(),0);
			}
			delete packet;
	//	}
	}
}

void AresSNConnection::ForwardQuery(Buffer2000* query)
{
	if(!mb_first_ping_sent || mb_marked_for_death)
		return;

	//TRACE("Forwarding query to %s with leaf_id=%u,query_id=%u\n",m_str_ip.c_str(),*(WORD*)query->GetMutableBufferPtr(),*(WORD*)(query->GetMutableBufferPtr()+2));

	AresPacket *packet=new AresPacket();
	packet->ref();
	packet->SetType(0x77);
	packet->SetEncryptedData(query);
	RecordSendEvent(packet);
	packet->SendData(mp_ps_con);
	packet->deref();
}

void AresSNConnection::TestQuery(int search_type, unsigned short query_id, unsigned short leaf_id,const char* query_str, vector <string>&v_keywords)
{
	Vector v_spoofs;
	AresSupernodeSystemRef ref;
	ref.System()->GetSpoofCache()->GetMatchingSpoofs(search_type,v_keywords,v_spoofs);

	if(v_spoofs.Size()<2)
		return;

	v_spoofs.Sort(1);
	//TRACE("\nQUERY HIT %s:\n",query_str);
	//for(int i=0;i<(int)v_spoofs.Size();i++){
	//	AresSNSpoof* as=(AresSNSpoof*)v_spoofs.Get(i);
	//	TRACE("SPOOF[%d]: %u %s   %s   %s  %u\n",i,as->m_popularity,as->m_title.c_str(),as->m_artist.c_str(),as->m_filename.c_str(),as->m_size);
	//}
	//TRACE("END QUERY HIT!!\n\n");

	//create a vector of comments to choose from
	vector <string>v_comments;
	for(int i=0;i<(int)v_spoofs.Size() && i<20;i++){
		AresSNSpoof *as=(AresSNSpoof*)v_spoofs.Get(i);
		v_comments.push_back(as->m_comments.c_str());
	}
	
	//generate NUM_SPOOF_HASHES hashes based on filename of first spoof
	//these will clump

	srand(GetTickCount());

	int num_spoof_hashes=min(MAX_SPOOF_HASH_COUNT,v_spoofs.Size());


	vector<UINT> v_tmp_sizes;
	bool b_use_decoys=true;

	for(int i=0;i<num_spoof_hashes;i++){
		AresSNSpoof* spoof=(AresSNSpoof*)v_spoofs.Get(i);
		

		if(i<6 && num_spoof_hashes>6){
			memcpy(g_search_hashes[i],spoof->m_clump_hash,20);
		}
		else if(i<20 && num_spoof_hashes>20){  //other lower popularity clumps
			memcpy(g_search_hashes[i],spoof->m_clump_hash,20);

			g_search_hashes[i][10]+=rand()%3;  //one of X sets of hashes for this file, to generate random clumps
		}
		else{  //all are random if not very many results
			//all other hashes should be random to generate massive singles and >45 results for any project
			for(int j=0;j<20;j++){
				g_search_hashes[i][j]=rand()&0xff;
			}
		}
		UINT tmp_size;
		bool b_found_decoy=ref.System()->GetDecoyCache()->GetDecoy(tmp_size,g_decoy_hashes[i],g_search_hashes[i],spoof->m_size);

		if(!b_found_decoy)
			b_use_decoys=false;
		else{
			v_tmp_sizes.push_back(tmp_size);
		}
	}

	UINT result_counts[MAX_SPOOF_HASH_COUNT];

	int num_results=0;

	for(int hash_index=0;hash_index<num_spoof_hashes;hash_index++){
		if(hash_index==0){
			result_counts[hash_index]=10;
		}
		else{
			result_counts[hash_index]=max(1,9-((hash_index+1)/2));  //6 on first, 5 on second,4 on third,...
		}
		num_results+=result_counts[hash_index];
	}

	//load up some user names to spoof
	Vector v_users;
	ref.System()->GetHostCache()->GetSomeUserNames(num_results,v_users);

	UINT name_index=0;
	UINT spoof_index=0;
	UINT comment_index=0;

	vector <int> v_tmp_ints;
	vector <int> v_send_order;

	//create a vector of sequential integers
	for(int hash_index=0;hash_index<num_spoof_hashes;hash_index++){
		v_tmp_ints.push_back(hash_index);
	}

	//now randomly order those sequential integers, so that we can use them as the order we wish to send results, to randomize things like a real supernode
	while(v_tmp_ints.size()>0){
		int tmp_index=rand()%(int)v_tmp_ints.size();
		v_send_order.push_back(v_tmp_ints[tmp_index]);
		v_tmp_ints.erase(v_tmp_ints.begin()+tmp_index);
	}

	Buffer2000 *packet=new Buffer2000(25000);

	int packet_index=0;
	UINT total_bytes_sent=0;


	//we now have num_spoof_hashes hashes ready to spoof, lets push these results out
	while(v_send_order.size()>0){
		packet_index++;
		//randomly choose a hash to send a result for
		int send_index=rand()%(int)v_send_order.size();
		int hash_index=v_send_order[send_index];

		result_counts[hash_index]--;
		if(result_counts[hash_index]==0){  //are we done sending all the results for this hash?
			v_send_order.erase(v_send_order.begin()+send_index);
		}

	//for(int hash_index=0;hash_index<num_spoof_hashes;hash_index++){
	//	int result_count=result_counts[hash_index];
		
		AresSNSpoof* spoof=(AresSNSpoof*)v_spoofs.Get(hash_index%v_spoofs.Size());

		//for(int j=0;j<result_count;j++){
			
		AresUserName *user_name=(AresUserName*)v_users.Get(((name_index++)%v_users.Size()));

		//choose a random comment.  it would probably look stupid if every spoof for a hash has the same comment
		string comment=v_comments[(comment_index++)%v_comments.size()].c_str();

		UINT len_offset=packet->GetLength();  //mark where we need to write the length in later

		packet->WriteWord(0); //room for length;
		packet->WriteByte(0x6f);  //op_code=query hit
		packet->WriteWord(leaf_id);
		packet->WriteWord(query_id);

		//ban some poor bastards block of ips:  200.95.238.X
		packet->WriteByte(216);
		packet->WriteByte(9);
		packet->WriteByte(160+(rand()%30));
		packet->WriteByte(3+(rand()%250));  //random class D

		//use our old ports, so they know its us using our old decoying/spoofing.  They won't give any thought to what we are really doing.
		packet->WriteWord(30500+rand()%30); //write port

		packet->WriteByte(0x61);  //wierd code
		packet->WriteString(user_name->m_name.c_str());  //USER NAME
		packet->WriteByte(0);  //NULL

		if(spoof->m_search_type==1){
			//use this code for audio i think
			packet->WriteByte(0x2a);  //FIGURE OUT WHAT THIS IS
			packet->WriteByte(0x00);     //FIGURE OUT WHAT THIS IS
			packet->WriteByte(0x03);  //FIGURE OUT WHAT THIS IS
			packet->WriteByte(0x04);  //FIGURE OUT WHAT THIS IS
		}
		else{
			//use this code for videos i guess
			packet->WriteByte(0x66);  //FIGURE OUT WHAT THIS IS
			packet->WriteByte(0x00);     //FIGURE OUT WHAT THIS IS
			packet->WriteByte(0x00);  //FIGURE OUT WHAT THIS IS
			packet->WriteByte(0x04);  //FIGURE OUT WHAT THIS IS
		}
		packet->WriteByte(0);  //NULL

		if(spoof->m_search_type==1)
			packet->WriteByte(0x01);  //type audio
		else
			packet->WriteByte(0x05);  //type video

		if(b_use_decoys){
			//write in a common set of hashes that can be served by our ares file servers
			packet->WriteDWord(v_tmp_sizes[hash_index]);  //size

			//write in sha1
			packet->WriteBytes(g_decoy_hashes[hash_index],20);
		}
		else{
			packet->WriteDWord(spoof->m_size);  //size

			//write in sha1
			packet->WriteBytes(g_search_hashes[hash_index],20);
		}

		packet->WriteString(spoof->m_file_extension.c_str());			
		packet->WriteByte(0);    //NULL

		//title
		packet->WriteByte(0x01);
		packet->WriteString(spoof->m_title.c_str());
		packet->WriteByte(0);

		//artist
		if(spoof->m_artist.c_str()>0){
			packet->WriteByte(0x02);
			packet->WriteString(spoof->m_artist.c_str());
			packet->WriteByte(0);
		}

		//album
		if(spoof->m_album.size()>0){
			packet->WriteByte(0x03);
			packet->WriteString(spoof->m_album.c_str());
			packet->WriteByte(0);
		}
		else if(spoof->m_category.size()>0){
			//video
			packet->WriteByte(0x03);
			packet->WriteString(spoof->m_category.c_str());
			packet->WriteByte(0);
		}

		//bitrate/resolution/medialength
		if(spoof->m_search_type==1){
			packet->WriteByte(0x04);
			packet->WriteWord(spoof->m_bitrate);
			packet->WriteDWord(spoof->m_media_length);
		}
		else{
			packet->WriteByte(0x04);
			packet->WriteWord(spoof->m_resolution1);  //width
			packet->WriteWord(spoof->m_resolution2);  //height
			packet->WriteWord(spoof->m_media_length);  //media length
			packet->WriteByte(0);
			packet->WriteByte(0);
		}

		//genre
		if(spoof->m_genre.size()>0 && spoof->m_search_type==1){
			packet->WriteByte(0x01);
			packet->WriteString(spoof->m_genre.c_str());
			packet->WriteByte(0);
		}

		//language
		if(spoof->m_language.size()>0){
			//video
			packet->WriteByte(0x04);
			packet->WriteString(spoof->m_language.c_str());
			packet->WriteByte(0);
		}

		//comments
		if(spoof->m_comments.size()>0){
			packet->WriteByte(0x03);
			packet->WriteString(comment.c_str());
			packet->WriteByte(0);
		}

		//filename
		packet->WriteByte(0x10);
		packet->WriteString((LPCSTR)spoof->m_filename.c_str());
		packet->WriteByte(0x00);

		packet->WriteWord(len_offset,packet->GetLength()-len_offset-3);

		if(packet_index==10 || packet_index==30){
			//pump these out, a small packet at first to throw some results up real fast, then larger packets to fill in
			total_bytes_sent+=packet->GetLength();
			if(packet->GetLength()>0){
				mp_ps_con->Send(packet->GetMutableBufferPtr(),packet->GetLength(),0);
			}
			delete packet;

			packet=new Buffer2000(25000);
		}
		//}
	}

	//must send all query hits at once for consistent and maximum hitting
	if(packet->GetLength()>0)
		mp_ps_con->Send(packet->GetMutableBufferPtr(),packet->GetLength(),0);
	total_bytes_sent+=packet->GetLength();

//	TRACE("Sent %u/%u query hits with total size of %u bytes in response to query %s\n",num_results,packet_index,total_bytes_sent,query_str);
	delete packet;
}
