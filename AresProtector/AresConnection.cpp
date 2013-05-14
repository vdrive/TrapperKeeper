#include "StdAfx.h"
#include "aresconnection.h"
#include "AresProtectionSystem.h"
#include "AresPacket.h"
#include "AresHost.h"
#include "AresUtilityClass.h"
//#include "AresConstants.h"
//#include <sha.h>
//#include "altsha.h"
#include ".\aresconnection.h"
#include "AresFile.h"
#include <mmsystem.h>
#include "..\AresDataCollector\ProcessorJob.h"

#include "zlib\zlib.h"

UINT g_id_counter=0;

AresConnection::AresConnection(AresHost* host,unsigned short my_port)
{
	m_my_port=my_port;
	Init(host->GetIP(),host->GetPort());

	//put all other init into the Init() function;
}

AresConnection::AresConnection(const char* host_ip,int port,unsigned short my_port)
{
	m_my_port=my_port;
	Init(host_ip,port);

	//put all other init into the Init() function;
}

void AresConnection::Init(const char *host_ip,unsigned short port){
	ZeroMemory(m_user_name,sizeof(m_user_name));
	m_network_users=0;
	m_network_files=0;
	m_network_MBs=0;
	m_original_con_handle=0;
	mb_clean=false;
	mb_0x38_client=false;

	AresProtectionSystemReference ref;
	ref.System()->UpdateConnecting(1);
	mb_connecting=true;

	mb_first_ping_sent=false;
	mb_shares_sent=false;
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

	m_state=GetTickCount();	
	mb_dead=false;
	m_id=++g_id_counter;
//	TRACE("AresConnection::AresConnection() id=%d\n",m_id);
	m_con_handle=NULL;
}

AresConnection::~AresConnection(void)
{	
	CleanUp();
}


void AresConnection::Update(void)
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


	if(mb_marked_for_death && !mb_dead){  //if Kill has been called
		int secs=(int)dead_time.GetTotalSeconds();
		if(secs>12){

			mb_dead=true;
			//AresProtectionSystemReference ref;

			//ref.System()->GetTCP()->CloseConnection(m_con);
			//ref.System()->GetTCP()->RemoveConnectionFromHashMap(this);  //time to die	
			return;  //this object doesn't even exist at this point
		}

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
			SendMessage0x00(job->m_sixteen_in);
			mv_processor_job.Remove(0);

			//CString log_msg;
			//log_msg.Format("Ares Connection:  %s Finished responding to 0x38 message.\n",m_str_ip.c_str());
			//AresDataCollectorSystemReference ref;
			//ref.System()->Log(log_msg);
		}
	}


	//we are connected and aren't marked for death
	if(mb_shares_sent && !mb_name_sent && alive_time.GetTotalSeconds()>10){
		SendName();
	}
	else if(mb_shares_sent && mb_name_sent && !mb_first_ping_sent && alive_time.GetTotalSeconds()>35){
		SendPing();
	}
	else if(mb_first_ping_sent && m_last_ping_sent.HasTimedOut(60)){
		SendPing();
	}

	
	/*
	if(mb_shares_sent && mv_shares_to_upload.Size()>0 && m_last_file_upload<now-CTimeSpan(0,0,0,30)){
		m_last_file_upload=now;
		SendShares(*(Vector*)mv_shares_to_upload.Get(0));
		mv_shares_to_upload.Remove(0);

		if(mv_shares_to_upload.Size()==0){
			CString tmp;
			tmp.Format("Finished uploading %d shares!",mv_files_shared.Size());
			m_status=(LPCSTR)tmp;
		}
	}*/
}

void AresConnection::Connect(void)
{
	//TRACE("AresConnection::Connect()  Trying to connect to %s:%u\n",m_str_ip.c_str(),m_port);

//	mp_ps_con=new PeerSocket();
//	mp_ps_con->Init(m_str_ip.c_str(),m_port,this);

	AresProtectionSystemReference ref;
	m_con_handle=ref.System()->GetTCPSystem()->OpenConnection(m_str_ip.c_str(),m_port);
	m_original_con_handle=m_con_handle;

//	if(stricmp("66.66.236.0",m_str_ip.c_str())==0){
//		int x=5;
//	}

	//AresProtectionSystemReference ref;
	
	//ref.System()->GetTCP()->OpenConnection(m_con);
	//ref.System()->GetTCP()->AddUltraPeerCon(this);
}

bool AresConnection::IsConnected(void)
{
	return mb_connected;
}

void AresConnection::Kill(void)
{
	mb_marked_for_death=true;
	m_death_time=CTime::GetCurrentTime();
	m_status+=" - Destroying";
	CleanUp();
}

CString AresConnection::GetFormattedAgeString(void)
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

void AresConnection::ProcessPacket(AresPacket& packet)
{
	int type=packet.GetMessageType();
	//TRACE("AresConnection::ProcessPacket() %s type 0x%X length=%u\n",m_str_ip.c_str(),type,packet.GetLength());

	if(packet.GetMessageType()==0x33){
		AresUtilityClass::DecryptFirstMessage(packet,m_remote_ip,m_fss,m_fsb,m_port,false);
		//byte *data=new byte[packet.GetLength()];
		//memcpy(data,packet.GetEncryptedData()->GetBufferPtr(),packet.GetLength());
		//DecryptFirstMessage(m_remote_ip,(unsigned char *)data,packet.GetLength());
		//packet.SetDecryptedData(new Buffer2000(data,packet.GetLength()));
		ExtractHosts(packet);

		unsigned char remote_guid[16];
		//memcpy(remote_guid,&data[2],sizeof(remote_guid));
		memcpy(remote_guid,packet.GetDecryptedData()->GetBufferPtr()+2,sizeof(remote_guid));

		// Use the remote guid to send back the message 0x00
		SendMessage0x00(remote_guid);

		//delete []data;
	}
	else if(packet.GetMessageType()==0x1){
		m_status="Received First Response, Connected";
		AresUtilityClass::DecryptMessage(packet,m_fss,m_fsb);

		m_network_users=*(unsigned int*)(packet.GetDecryptedData()->GetBufferPtr()+2);
		m_network_files=*(unsigned int*)(packet.GetDecryptedData()->GetBufferPtr()+2+4);
		m_network_MBs=*(unsigned int*)(packet.GetDecryptedData()->GetBufferPtr()+2+4+4);
		TRACE("AresConnection::ProcessPacket(AresPacket& packet) message 0x1 %u users, %u files, %u MBs\n",m_network_users,m_network_files,m_network_MBs);
		AresProtectionSystemReference ref;
		ref.System()->GetHostCache()->AddHosts(mv_hosts);
	}
	else if(packet.GetMessageType()==0x05){
		AresUtilityClass::DecryptMessage(packet,m_fss,m_fsb);
		byte *data=packet.GetDecryptedData()->GetMutableBufferPtr();
		UINT dlen=packet.GetDecryptedData()->GetLength();

		//TRACE("\n\n%s PACKET 0x%X BREAKDOWN\n",m_str_ip.c_str(),packet.GetMessageType());
		//for(int i=0;i<(int)dlen;i++){
		//	byte b=data[i];

		//	if(b!=0){
		//		TRACE("%d 0x%.2X '%c' \n",i,b,b);
		//	}
		//	else{
		//		TRACE("%d 0x%.2X NULL \n",i,b,b);
		//	}
		//}
		//TRACE("\n\n");
		if(!mb_shares_sent)
			SendShares();
	}
	else if(packet.GetMessageType()==0x38){
		mb_0x38_client=true;
		AresUtilityClass::DecryptFirstMessage(packet,m_remote_ip,m_fss,m_fsb,m_port,true);
		//byte *data=new byte[packet.GetLength()];
		//memcpy(data,packet.GetEncryptedData()->GetBufferPtr(),packet.GetLength());
		//DecryptFirstMessage(m_remote_ip,(unsigned char *)data,packet.GetLength());
		//packet.SetDecryptedData(new Buffer2000(data,packet.GetLength()));
		ExtractHosts(packet);

		unsigned char remote_guid[16];
		//memcpy(remote_guid,&data[2],sizeof(remote_guid));
		memcpy(remote_guid,packet.GetDecryptedData()->GetBufferPtr()+2,sizeof(remote_guid));

		// Use the remote guid to send back the message 0x00
		
		Begin0x38Response(remote_guid);

		//delete []data;
	}
	else if(packet.GetMessageType()==0x25){
		AresUtilityClass::DecryptMessage(packet,m_fss,m_fsb);
		byte *data=packet.GetDecryptedData()->GetMutableBufferPtr();
		UINT dlen=packet.GetDecryptedData()->GetLength();

		//TRACE("\n\n%s PACKET 0x%X BREAKDOWN\n",m_str_ip.c_str(),packet.GetMessageType());
		//for(int i=0;i<(int)dlen;i++){
		//	byte b=data[i];

		//	if(b!=0){
		//		TRACE("%d 0x%.2X '%c' \n",i,b,b);
		//	}
		//	else{
		//		TRACE("%d 0x%.2X NULL \n",i,b,b);
		//	}
		//}
		//TRACE("\n\n");
	}
	else if(packet.GetMessageType()==0x1e){
		AresUtilityClass::DecryptMessage(packet,m_fss,m_fsb);
		byte *data=packet.GetDecryptedData()->GetMutableBufferPtr();
		UINT dlen=packet.GetDecryptedData()->GetLength();

		//TRACE("\n\n%s PACKET 0x%X BREAKDOWN\n",m_str_ip.c_str(),packet.GetMessageType());
		//for(int i=0;i<(int)dlen;i++){
		//	byte b=data[i];

		//	if(b!=0){
		//		TRACE("%d 0x%.2X '%c' \n",i,b,b);
		//	}
		//	else{
		//		TRACE("%d 0x%.2X NULL \n",i,b,b);
		//	}
		//}
		//TRACE("\n\n");
	}
	else if(packet.GetMessageType()==0x3a){
		AresUtilityClass::DecryptMessage(packet,m_fss,m_fsb);
		byte *data=packet.GetDecryptedData()->GetMutableBufferPtr();
		UINT dlen=packet.GetDecryptedData()->GetLength();

		//TRACE("\n\n%s PACKET 0x%X BREAKDOWN\n",m_str_ip.c_str(),packet.GetMessageType());
		//for(int i=0;i<(int)dlen;i++){
		//	byte b=data[i];

		//	if(b!=0){
		//		TRACE("%d 0x%.2X '%c' \n",i,b,b);
		//	}
		//	else{
		//		TRACE("%d 0x%.2X NULL \n",i,b,b);
		//	}
		//}
		//TRACE("\n\n");
	}
	else if(packet.GetMessageType()==0x37){
		AresUtilityClass::DecryptMessage(packet,m_fss,m_fsb);
		byte *data=packet.GetDecryptedData()->GetMutableBufferPtr();
		UINT dlen=packet.GetDecryptedData()->GetLength();

		//TRACE("\n\n%s PACKET 0x%X BREAKDOWN\n",m_str_ip.c_str(),packet.GetMessageType());
		//for(int i=0;i<(int)dlen;i++){
		//	byte b=data[i];

		//	if(b!=0){
		//		TRACE("%d 0x%.2X '%c' \n",i,b,b);
		//	}
		//	else{
		//		TRACE("%d 0x%.2X NULL \n",i,b,b);
		//	}
		//}
		//TRACE("\n\n");
	}
	else{
		//int type=packet.GetMessageType();
		//TRACE("PROCESSING PACKET OF TYPE 0x%X\n",type);
	}
}

void AresConnection::ExtractHosts(AresPacket& packet)
{
	int host_data_length=packet.GetLength()-16-3-2;
	int host_count=host_data_length/6;
	TRACE("AresConnection::ExtractHosts() Extraction %d hosts.\n",host_count);
	const byte *ptr=packet.GetDecryptedData()->GetBufferPtr()+16+3+2;
	for(int i=0;i<host_count;i++)
	{
		AresHost *new_host=new AresHost(*(unsigned int*)ptr,*(unsigned short*)(ptr+4));
		mv_hosts.Add(new_host);
		ptr+=6;
	}

	//p_parent->m_host_cache.AddHosts(hosts);
	//ExtractHosts((unsigned char *)&data[2+16+3],m_hdr.Len()-2-16-3);
}

LinkedList* AresConnection::GetEventList(void)
{
	return &m_event_list;
}

void AresConnection::SendMessage0x00(byte* remote_guid)
{
	static bool in_function=false;

	ASSERT(!in_function);

	in_function=true;

	Buffer2000 *p_unencrypted_data=new Buffer2000();
	Buffer2000 *p_encrypted_data=new Buffer2000();

	// Calculate the sha1 to send back from the remote guid
	unsigned char sha1[20];
	if(mb_0x38_client){
		ProcessorJob* job=(ProcessorJob*)mv_processor_job.Get(0);
		memcpy(sha1,job->m_twenty_out,20);
	}
	else{
		AresUtilityClass::CalculateRemoteGUIDSha1(remote_guid,sha1,m_fss,m_fsb);
	}
	//ThirtyThreeSixteenToZeroTwenty(remote_guid,sha1);

	// Get the 2 bytes to go on the end of the 0x00 20, finishing up the 0x00 22
	
	unsigned short int twenty_two=0;
	
	if(mb_0x38_client==false)
		twenty_two=AresUtilityClass::CalculateTwoBytesFromBuffer(sha1,sizeof(sha1));//CalculateTwoBytesFromBuffer(sha1,sizeof(sha1));

	// Calculate the encryption constants
	unsigned char encryption_byte=AresUtilityClass::CalcStateByte(0xFE,m_state)+1;
	unsigned short int encryption_short=AresUtilityClass::GetEncryptionShort(m_fsb);//*((unsigned short int *)&const_591da8[m_fsb*2]);
	unsigned short int piss_encryption_short=AresUtilityClass::FourPissAlgorithm(m_fss,encryption_byte,encryption_short);//FourPissAlgorithm(m_fss,encryption_byte,encryption_short);
	unsigned char check_byte=AresUtilityClass::CalcStateByte(0xFA,m_state)+1;	

	// Form the 0x00 message (username monkeyfarts is in there)
							 //0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56   57   58   59   60   61   62   63   64   65   66   67   68   69   70   71   72   73   74   75   76   77   78                                                                                                          || listening port  ||                                  
	static byte peer0_1a[] = {0x4c,0x00,0x00,0x96,0x2a,0x00,0x01,0x38,0xa2,0x8f,0xc8,0x10,0xdd,0x82,0x25,0xde,0x81,0x06,0xd1,0xdc,0x4e,0xc0,0xcf,0x9a,0x82,0xf6,0x87,0x8d,0x00,0x00,0x00,0x04,0x00,0x00,0x9b,0x06,0x6d,0x6f,0x6e,0x6b,0x65,0x79,0x66,0x61,0x72,0x74,0x73,0x00,0x6f,0x9f,0x41,0x21,0xb8,0x8c,0x12,0x43,0xa4,0xce,0x9b,0x43,0xd7,0x31,0xc9,0x47,0x00,0x00,0x41,0x72,0x65,0x73,0x4c,0x69,0x74,0x65,0x00,0x26,0x77,0x42,0x24};
	static byte peer0_1b[] = {0x45,0x00,0x00,0x96,0x2a,0x01,0x38,0xa2,0x8f,0xc8,0x10,0xdd,0x82,0x25,0xde,0x81,0x06,0xd1,0xdc,0x4e,0xc0,0xcf,0x9a,0x82,0xf6,0x00,0x00,0x00,0x01,0x00,0x00,0x67,0x5b,0x6d,0x6f,0x6e,0x6b,0x65,0x79,0x66,0x61,0x72,0x74,0x73,0x00,0x6f,0x9f,0x41,0x21,0xb8,0x8c,0x12,0x43,0xa4,0xce,0x9b,0x43,0xd7,0x31,0xc9,0x47,0x00,0x00,0x41,0x72,0x65,0x73,0x00,0x26,0x77,0x42,0x24};

	int fd=rand()%7;
	int sd=rand()%7;
	int td=fd+sd;

	/*sprintf(m_user_name,"anon_%x%x%x%03x",fd,sd,td,rand() % 0xFFF);*/
	sprintf(m_user_name,"blaha%x%x%x%03x",fd,sd,td,rand() % 0xFFF);
	
	p_unencrypted_data->WriteByte(encryption_byte);  //0
	p_unencrypted_data->WriteByte(check_byte); //1

	if(mb_0x38_client==false)
		p_unencrypted_data->WriteBytes(peer0_1a+5,sizeof(peer0_1a)-5);  //copy that stuff up there into our unencrypted buffer
	else
		p_unencrypted_data->WriteBytes(peer0_1b+5,sizeof(peer0_1b)-5);  //copy that stuff up there into our unencrypted buffer
	
	//acquire a mutable pointer from our unencrypted data buffer so that we may change data inside of it via the ptr
	byte *dp=p_unencrypted_data->GetMutableBufferPtr();
	
	//copy the sha into there
	//memcpy(&msg[6],sha1,sizeof(sha1));	// copy the 20
	if(mb_0x38_client == false)
		memcpy(dp+6-3,sha1,sizeof(sha1));
	else
		memcpy(dp+6-3-1,sha1,sizeof(sha1));


	if(mb_0x38_client == false)
		memcpy(dp+26-3,&twenty_two,2);		// copy the 2

	//copy the user name into there
	if(mb_0x38_client == false)
		memcpy(dp+0x24-3,m_user_name,sizeof(m_user_name));
	else
		memcpy(dp+0x24-3-3,m_user_name,sizeof(m_user_name));

	if(mb_0x38_client == false)
		*(unsigned short*)(dp+0x24-3-2)=m_my_port;  //set my port
	else
		*(unsigned short*)(dp+0x24-3-2-3)=m_my_port;  //set my port

	AresProtectionSystemReference ref;
	string my_ip=ref.System()->GetSystemIP();
	unsigned int ip1=0,ip2=0,ip3=0,ip4=0;
	sscanf(my_ip.c_str(),"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	if(ip1==38 && ip2==119 && ip3==66 && ip4==45)
		ip4=24;  //fool the client for local testing

	if(mb_0x38_client == false){
		*(dp+0x4b-3)=(byte)ip1;
		*(dp+0x4c-3)=(byte)ip2;
		*(dp+0x4d-3)=(byte)ip3;	
		*(dp+0x4e-3)=(byte)ip4;
	}
	else{
		*(dp+0x4b-3-7)=(byte)ip1;
		*(dp+0x4c-3-7)=(byte)ip2;
		*(dp+0x4d-3-7)=(byte)ip3;	
		*(dp+0x4e-3-7)=(byte)ip4;
	}

	// Randomize my GUID
	for(int i=0;i<16;i++)
	{
		//msg[0x30+i]=rand() % 256;
		if(mb_0x38_client == false)
			*(dp+0x30+i-3)=rand() % 256;
		else
			*(dp+0x30+i-3-3)=rand() % 256;
	}		

	//done creating the data, lets encrypt it and send it

	//copy the user name into there
//	if(mb_0x38_client == false)
//		memcpy(dp+0x24-3,m_user_name,sizeof(m_user_name));
//	else
//		memcpy(dp+0x24-3-3,m_user_name,sizeof(m_user_name));

	//copy the unencrypted data into the encrypted buffer
	//also fix the user name to not be anon_XXXXXX by injecting it into the data in place of the other
	if(mb_0x38_client == false){
		p_encrypted_data->WriteBytes(p_unencrypted_data->GetBufferPtr(),0x24-3);
		p_encrypted_data->WriteString(ref.System()->GetUserName(m_my_port).c_str());
		p_encrypted_data->WriteByte(0);

		//append the rest of the data
		p_encrypted_data->WriteBytes(p_unencrypted_data->GetBufferPtr()+0x24-3+12,p_unencrypted_data->GetLength()-(0x24-3)-12);  //append remaining buffer on
	}
	else{
		p_encrypted_data->WriteBytes(p_unencrypted_data->GetBufferPtr(),0x24-3-3);
		p_encrypted_data->WriteString(ref.System()->GetUserName(m_my_port).c_str());
		p_encrypted_data->WriteByte(0);

		//append the rest of the data
		p_encrypted_data->WriteBytes(p_unencrypted_data->GetBufferPtr()+0x24-3-3+12,p_unencrypted_data->GetLength()-(0x24-3-3)-12);  //append remaining buffer on
	}

	//go back and fix the name in the unencrypted buffer so we can view it properly
	p_unencrypted_data->Clear();
	p_unencrypted_data->WriteBytes(p_encrypted_data->GetBufferPtr(),p_encrypted_data->GetLength());

	//encrypt the data in the encrypted buffer
	AresUtilityClass::EncryptMessage(p_encrypted_data->GetMutableBufferPtr()+2,p_encrypted_data->GetLength()-2,piss_encryption_short);

	//create an ares packet so we can record this event for debugging purposes
	AresPacket *new_packet=new AresPacket();
	new_packet->SetType(0x00); //message 00
	new_packet->SetEncryptedData(p_encrypted_data);
	new_packet->SetDecryptedData(p_unencrypted_data);

	new_packet->SendData(m_con_handle,ref.System()->GetTCPSystem());
	RecordSendEvent(new_packet);
	//and finally send our message
	//if(mp_ps_con){
	//	new_packet->SendData(mp_ps_con);
	//	RecordSendEvent(new_packet);
	//}
	//else{
	//	delete new_packet;
	//}
	in_function=false;
}

void AresConnection::OnClose(){
	TRACE("AresConnection::OnClose() %s:%u\n",m_str_ip.c_str(),m_port);
	mb_connected=false;
	if(mb_has_connected){
		m_status="Connection Lost";
		if(mb_shares_sent)
			m_status+=", Files Sent";
	}
	else
		m_status="Could Not Connect";
	Kill();
}

void AresConnection::OnSend(){
	//TRACE("AresConnection::OnSend() %s\n",m_str_ip.c_str());
}

void AresConnection::OnReceive(byte *data,UINT length){

	if(length>0)
		m_last_receive.Refresh();

	//TRACE("AresConnection::OnReceive() %s received %d bytes of data\n",m_str_ip.c_str(),length);
	m_receive_buffer.WriteBytes(data,length);

	while(true){  //multiple packets may come in a single OnReceive call
		AresPacket *packet=new AresPacket();
		if(packet->Read(&m_receive_buffer)){
			Vector v_tmp;
			v_tmp.Add(packet);
			RecordReceiveEvent(packet);
			ProcessPacket(*packet);
		}
		else{
			delete packet;
			break;
		}
	}
}

void AresConnection::OnConnect(){
	TRACE("AresConnection::OnConnect %s:%u CONNECTION SUCCESSFUL\n",m_str_ip.c_str(),m_port);
	//m_create_time=CTime::GetCurrentTime();
	AresProtectionSystemReference ref;

	unsigned char data[]={0x03,0x00,0x5a,0x04,0x03,0x05};
	//if(mp_ps_con)
	//	mp_ps_con->Send(data,sizeof(data));
	ref.System()->GetTCPSystem()->SendData(m_con_handle,data,sizeof(data));
	//ref.System()->GetTCP()->SendData(m_con,data,sizeof(data)); //send the initialization data
	mb_has_connected=true;
	mb_connected=true;
	m_status="Connected - sent init packet";

	if(mb_connecting){
		AresProtectionSystemReference ref;
		ref.System()->UpdateConnecting(-1);
		mb_connecting=false;
	}
}

void AresConnection::OnConnectFailed(){
//	TRACE("AresConnection::OnConnectFailed() %s\n",m_str_ip.c_str());
	mb_connected=false;
	m_status="Could Not Connect";
	Kill();
}

void AresConnection::CleanUp(void)
{
	if(m_con_handle!=NULL){
		AresProtectionSystemReference ref;
		ref.System()->GetTCPSystem()->CloseConnection(m_con_handle);
		m_con_handle=NULL;
	}
/*
	if(mp_ps_con){
		
		if(mp_ps_con->m_hSocket!=INVALID_SOCKET){
			SOCKET s=mp_ps_con->Detach();
			closesocket(s);
		}

		//mp_ps_con->Close();
		//mp_ps_con->ShutDown
		delete mp_ps_con;
	}
	mp_ps_con=NULL;*/

	if(mb_connecting){
		AresProtectionSystemReference ref;
		ref.System()->UpdateConnecting(-1);
		mb_connecting=false;
	}

	mb_clean=true;
	//_ASSERTE( _CrtCheckMemory( ) );
}

void AresConnection::SendShares(void)
{
	m_last_file_upload=CTime::GetCurrentTime();
	mb_shares_sent=true;
	m_num_shares=0;

	AresProtectionSystemReference ref;
	Vector v_file_group;
	ref.System()->GetSomeSpoofs(v_file_group);
	if(v_file_group.Size()==0)
		return;

	Vector *v_spoofs=(Vector*)v_file_group.Get(0);

	srand(timeGetTime());

	
	CString tmp;

	mv_files_shared.Clear();
	mv_files_shared.Add(v_spoofs);//Copy(&v_spoofs);
	//SendShares(v_spoofs);
	
	
	int num_files_uploaded=0;
	Vector v_shares_to_upload;

	while(num_files_uploaded<(int)v_spoofs->Size()){
		Vector *v_tmp_files=new Vector();
		for(int i=0;i<35 && num_files_uploaded<(int)v_spoofs->Size();i++){
			v_tmp_files->Add(v_spoofs->Get(num_files_uploaded++));
		}

		v_shares_to_upload.Add(v_tmp_files);
	}

	//upload first groups of files, others will be uploaded from update function
	while(v_shares_to_upload.Size()>0){
		m_last_file_upload=CTime::GetCurrentTime();
		SendShares(*(Vector*)v_shares_to_upload.Get(0));
		v_shares_to_upload.Remove(0);
	}

	tmp.Format("Uploading file list: %d files total",v_spoofs->Size());
	m_status=(LPCSTR)tmp;
}

//0x1e
void AresConnection::SendPing(void)
{
	Buffer2000 *p_unencrypted_data=new Buffer2000();
	Buffer2000 *p_encrypted_data=new Buffer2000();

	mb_first_ping_sent=true;
	m_last_ping_sent.Refresh();
//	unsigned int buflen=3+2+7;
//	unsigned char *buf=new unsigned char[buflen];
//	memset(buf,0,buflen);

//	*(unsigned short*)buf=buflen-3;  //length of packet
//	buf[2]=0x1e;  //ping

	// Set data
//	buf[6]=0x04; 

	// Calculate the encryption constants
	unsigned char encryption_byte=AresUtilityClass::CalcStateByte(0xFE,m_state)+1;
	unsigned short int encryption_short=AresUtilityClass::GetEncryptionShort(m_fsb);//*((unsigned short int *)&const_591da8[m_fsb*2]);
	unsigned short int piss_encryption_short=AresUtilityClass::FourPissAlgorithm(m_fss,encryption_byte,encryption_short);
	unsigned char check_byte=AresUtilityClass::CalcStateByte(0xFA,m_state)+1;	

	p_unencrypted_data->WriteByte(encryption_byte);  //0
	p_unencrypted_data->WriteByte(check_byte); //1
	p_unencrypted_data->WriteByte(0); //2
	p_unencrypted_data->WriteByte(0x04); //3
	p_unencrypted_data->WriteByte(0); //4
	p_unencrypted_data->WriteByte(0); //5
	p_unencrypted_data->WriteByte(0); //6
	p_unencrypted_data->WriteByte(0); //7
	p_unencrypted_data->WriteByte(0); //8

	//copy the unencrypted data into the encrypted buffer
	p_encrypted_data->WriteBytes(p_unencrypted_data->GetBufferPtr(),p_unencrypted_data->GetLength());

	//encrypt the data in the encrypted buffer
	AresUtilityClass::EncryptMessage(p_encrypted_data->GetMutableBufferPtr()+2,p_encrypted_data->GetLength()-2,piss_encryption_short);

	//create an ares packet so we can record this event for debugging purposes
	AresPacket *new_packet=new AresPacket();
	new_packet->SetType(0x1e); //ping
	new_packet->SetEncryptedData(p_encrypted_data);
	new_packet->SetDecryptedData(p_unencrypted_data);

//	if(stricmp(m_str_ip.c_str(),"66.66.236.0")==0){
//		int x=5;
//	}

	AresProtectionSystemReference ref;
	new_packet->SendData(m_con_handle,ref.System()->GetTCPSystem());
	RecordSendEvent(new_packet);
	//if(mp_ps_con){
	//	new_packet->SendData(mp_ps_con);
	//	RecordSendEvent(new_packet);
	//}
	//else{
	//	delete new_packet;
	//}
}

//0x22
void AresConnection::SendName(void)
{
	Buffer2000 *p_unencrypted_data=new Buffer2000();
	Buffer2000 *p_encrypted_data=new Buffer2000();

	mb_name_sent=true;

	// Calculate the encryption constants
	unsigned char encryption_byte=AresUtilityClass::CalcStateByte(0xFE,m_state)+1;
	unsigned short int encryption_short=AresUtilityClass::GetEncryptionShort(m_fsb);//*((unsigned short int *)&const_591da8[m_fsb*2]);
	unsigned short int piss_encryption_short=AresUtilityClass::FourPissAlgorithm(m_fss,encryption_byte,encryption_short);
	unsigned char check_byte=AresUtilityClass::CalcStateByte(0xFA,m_state)+1;	

	p_unencrypted_data->WriteByte(encryption_byte);
	p_unencrypted_data->WriteByte(check_byte);
	AresProtectionSystemReference ref;
	p_unencrypted_data->WriteString(ref.System()->GetUserName(m_my_port).c_str());

	//copy the unencrypted data into the encrypted buffer
	p_encrypted_data->WriteBytes(p_unencrypted_data->GetBufferPtr(),p_unencrypted_data->GetLength());

	//encrypt the data in the encrypted buffer
	AresUtilityClass::EncryptMessage(p_encrypted_data->GetMutableBufferPtr()+2,p_encrypted_data->GetLength()-2,piss_encryption_short);

	//create an ares packet so we can record this event for debugging purposes
	AresPacket *new_packet=new AresPacket();
	new_packet->SetType(0x22); //name
	new_packet->SetEncryptedData(p_encrypted_data);
	new_packet->SetDecryptedData(p_unencrypted_data);

	new_packet->SendData(m_con_handle,ref.System()->GetTCPSystem());
	RecordSendEvent(new_packet);

	//if(mp_ps_con){
	//	new_packet->SendData(mp_ps_con);
	//	RecordSendEvent(new_packet);
	//}
	//else{
	//	delete new_packet;
	//}
}

void AresConnection::RecordReceiveEvent(AresPacket* packet)
{
	CSingleLock lock(&m_event_list.m_list_lock,TRUE);
	AresProtectionSystemReference ref;
	if(ref.System()->GetEventCacheTime()>0)
		m_event_list.Add(packet);
	else delete packet;
}

void AresConnection::RecordSendEvent(AresPacket* packet)
{
	CSingleLock lock(&m_event_list.m_list_lock,TRUE);
	AresProtectionSystemReference ref;
	if(ref.System()->GetEventCacheTime()>0)
		m_event_list.Add(packet);
	else delete packet;
}

//call to remove events that have expired past the cache time
void AresConnection::PurgeEvents(void)
{
	if(m_event_list.Size()==0)
		return;
	m_event_list.StartIteration();
	AresProtectionSystemReference ref;
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

bool AresConnection::IsSharingFiles(void)
{
	return mb_shares_sent;
}

int AresConnection::SendShares(Vector& v_files)
{
	if(v_files.Size()<1)
		return 0;

	unsigned int buf_len=0;

	//Build BUffer
	Buffer2000 output;

	//unsigned int original_seed=(unsigned int)timeGetTime();

	// Form message
//	unsigned char *ptr=buf;
	for(int file_index=0;file_index<(int)v_files.Size();file_index++){
		m_num_shares++;
		AresFile *the_file=(AresFile*)v_files.Get(file_index);

		int start_offset=output.GetLength();
		
		output.WriteWord(0);  //set size of this share later = length of buf-start_offset-3
		output.WriteByte(0x1c);  //new share?

		output.WriteWord(the_file->m_keywords_length+(4*((int)the_file->mv_keyword_title.size()+(int)the_file->mv_keyword_artist.size())));  //length of keywords  //*((unsigned short int *)ptr)=keywords_length+(4*num_keywords);	// length of keywords

		// Keywords
		for(int i=0;i<(int)the_file->mv_keyword_title.size();i++)
		{
			// keyword type = title
			output.WriteByte(0x01);				
			// keyword checksum
			output.WriteWord(AresUtilityClass::CalculateTwoBytesFromBuffer((unsigned char *)the_file->mv_keyword_title[i].c_str(),(int)the_file->mv_keyword_title[i].size()));
	
			// keyword length
			output.WriteByte((byte)the_file->mv_keyword_title[i].size());

			// keyword
			output.WriteString(the_file->mv_keyword_title[i].c_str());
		}
		for(int i=0;i<(int)the_file->mv_keyword_artist.size();i++)
		{
			// keyword type = artist
			output.WriteByte(0x02);
							
			// keyword checksum
			output.WriteWord(AresUtilityClass::CalculateTwoBytesFromBuffer((unsigned char *)the_file->mv_keyword_artist[i].c_str(),(int)the_file->mv_keyword_artist[i].size()));
			
			// keyword length
			output.WriteByte((byte)the_file->mv_keyword_artist[i].size());

			output.WriteString(the_file->mv_keyword_artist[i].c_str());
			// keyword
		}

		// Bitrate, sampling rate, length in seconds
		output.WriteDWord(192);  //bitrate
		output.WriteDWord(44100);  //sampling rate
		output.WriteDWord(the_file->m_media_length);  //1:23		

		output.WriteByte(the_file->m_type);  //type audio/video

		output.WriteDWord(the_file->m_size);  //file size

		//Sha-1
		output.WriteBytes(the_file->m_hash,20);  //TYDEBUG
		//for(int tmp_index=0;tmp_index<20;tmp_index++)  //write in completely random hashes, because we only want to generate singles
		//	output.WriteByte(rand()&0xff);

		// Extension
		output.WriteString(the_file->m_file_extension.c_str());
		output.WriteByte(0);

		// Meta tags
		//title
		output.WriteByte(0x01);
		output.WriteString(the_file->m_meta_title.c_str());
		output.WriteByte(0);

		output.WriteByte(0x02);
		output.WriteString(the_file->m_meta_artist.c_str());
		output.WriteByte(0);

		//this would be the resolution
		output.WriteByte(0x04);  //bitrate again

		if(the_file->m_type==5){
			//this field is resolution for movies
			output.WriteWord(352);  //width
			output.WriteWord(240);  //height
			output.WriteWord(60*60+30*60+(rand()%(60*60)));  //media length  - between 1.5 hours and 2.5 hours in length
			output.WriteByte(0);
			output.WriteByte(0);
		}
		else{
			output.WriteWord(192);  
			output.WriteDWord(the_file->m_media_length);  //media length again
		}
		
		output.WriteByte(0x10);
		output.WriteString(the_file->m_file_name.c_str());
		output.WriteByte(0);

		output.WriteWord(start_offset,output.GetLength()-start_offset-3);
	}

	unsigned int comp_len=output.GetLength()+1024;	// just to be safe
	unsigned char *comp_buf=new unsigned char[3+comp_len];
	memset(comp_buf,0,3+comp_len);

	//compress ((Bytef *)compressed_data, (uLongf *)&compressed_length, (const Bytef *)data, (uLong) data_length);
	compress ((Bytef *)&comp_buf[3], (uLongf *)&comp_len, (const Bytef *)output.GetBufferPtr(), (uLong) output.GetLength());

	comp_buf[2]=0x32;  //send shares
	*(unsigned short*)comp_buf=comp_len;

	SendData(comp_buf,3+comp_len);
	//if(mp_ps_con)
	//	mp_ps_con->Send(comp_buf,3+comp_len);
	
	AresPacket *new_packet=new AresPacket();
	new_packet->SetType(0x32); //message 32 - send shares
	RecordSendEvent(new_packet);

	delete [] comp_buf;

	return (int)v_files.Size();
}

void AresConnection::GetSharedFiles(Vector& v)
{
	v.Copy(&mv_files_shared);
}

void AresConnection::Begin0x38Response(byte* remote_guid)
{
	ProcessorJob *new_job=new ProcessorJob(remote_guid);
	mv_processor_job.Add(new_job);
	AresProtectionSystemReference ref;
	ref.System()->AddProcessorJob(new_job);  //let the system begin crunching on this one
}

void AresConnection::SendData(byte* data, UINT length)
{
	AresProtectionSystemReference ref;
	ref.System()->GetTCPSystem()->SendData(m_con_handle,data,length);
}

UINT AresConnection::GetConHandle(void)
{
	return m_con_handle;
}
