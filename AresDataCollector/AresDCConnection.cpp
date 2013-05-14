#include "StdAfx.h"
#include "aresdcconnection.h"
#include "AresDataCollectorSystem.h"
#include "..\AresProtector\AresPacket.h"
#include "..\AresProtector\AresHost.h"
#include "..\AresProtector\AresUtilityClass.h"
//#include "AresConstants.h"
//#include <sha.h>
//#include "altsha.h"
#include "..\AresProtector\AresFile.h"
#include "AresSearchResult.h"
#include <mmsystem.h>

#include "zlib\zlib.h"

UINT g_id_counter=0;

AresDCConnection::AresDCConnection(AresHost* host,unsigned short my_port)
{
	mv_host.Add(host);
	m_my_port=my_port;
	m_int_ip=host->GetIntIP();
	Init(host->GetIP(),host->GetPort());
	//put all other init into the Init() function;
}


/*
AresDCConnection::AresDCConnection(const char* host_ip,int port,unsigned short my_port)
{
	m_my_port=my_port;
	m_int_ip=host->GetIntIP();
	Init(host_ip,port);
	//put all other init into the Init() function;
}*/

void AresDCConnection::Init(const char *host_ip,unsigned short port){

	mb_searched=false;

	mb_0x38_client=false;

	ZeroMemory(m_user_name,sizeof(m_user_name));
	m_network_users=0;
	m_network_files=0;
	m_network_MBs=0;

	mb_clean=false;

	AresDataCollectorSystemReference ref;
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
//	TRACE("AresDCConnection::AresDCConnection() id=%d\n",m_id);
	mp_ps_con=NULL;
}

AresDCConnection::~AresDCConnection(void)
{	
	CleanUp();
}


void AresDCConnection::Update(void)
{
	if(mb_dead)
		return;

	if(m_last_receive.HasTimedOut(1200) && !mb_marked_for_death){  //10 minutes of waiting then we drop
		m_status="idle";
		Kill();
		return;
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

			CString log_msg;
			log_msg.Format("Ares Connection:  %s Finished responding to 0x38 message.\n",m_str_ip.c_str());
			AresDataCollectorSystemReference ref;
			ref.System()->Log(log_msg);
		}
	}

	//we are connected and aren't marked for death
	if(mb_shares_sent && !mb_name_sent && alive_time.GetTotalSeconds()>10){
		SendName();
	}
	else if(mb_shares_sent && mb_name_sent && !mb_first_ping_sent && alive_time.GetTotalSeconds()>30){
		SendPing();
	}
	else if(mb_shares_sent && mb_name_sent && mb_first_ping_sent && m_last_ping_sent.HasTimedOut(60)){
		SendPing();
	}

	//do searching
	if(!mb_searched && mb_shares_sent && mb_name_sent && mb_first_ping_sent){
		CSingleLock lock(&m_search_lock,TRUE);
		if(mv_search_project.Size()>0){
			mb_searched=true;
			m_last_search.Refresh();
			AresDCProjectInfo *proj=(AresDCProjectInfo*)mv_search_project.Get(0);
			Search(proj->m_name.c_str(),proj->mv_search_keywords);

			AresDataCollectorSystemReference ref;
			CString log_msg;
			log_msg.Format("Ares Client: %s running search '%s' on project '%s'",m_str_ip.c_str(),proj->m_search_string.c_str(),proj->m_name.c_str());
			ref.System()->Log(log_msg);
		}
	}
}

void AresDCConnection::Connect(void)
{
	//TRACE("AresDCConnection::Connect()  Trying to connect to %s:%u\n",m_str_ip.c_str(),m_port);
	mp_ps_con=new PeerSocket();
	mp_ps_con->Init(m_str_ip.c_str(),m_port,this);
	//AresProtectionSystemReference ref;
	
	//ref.System()->GetTCP()->OpenConnection(m_con);
	//ref.System()->GetTCP()->AddUltraPeerCon(this);
}

bool AresDCConnection::IsConnected(void)
{
	return mb_connected;
}

void AresDCConnection::Kill(void)
{
	mb_marked_for_death=true;
	m_death_time=CTime::GetCurrentTime();
	m_status+=" - Destroying";
	CleanUp();

	if(!mb_shares_sent){
		//increment this hosts fail count if we were never able to upload files.
		//this will allow us to purge the more useless hosts so we can focus on the successful hosts
		AresHost* host=(AresHost*)mv_host.Get(0);
		host->m_fail_count++;
	}

}

CString AresDCConnection::GetFormattedAgeString(void)
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

void AresDCConnection::ProcessPacket(AresPacket& packet)
{

	int type=packet.GetMessageType();
	CString log_msg;
	/*
	log_msg.Format("Ares Connection:  %s PROCESSING PACKET OF TYPE 0x%X\n",m_str_ip.c_str(),type);
	AresDataCollectorSystemReference ref;
	ref.System()->Log(log_msg);
	*/

	/*  //TURN OFF 0x33 for debugging 0x38
	if(packet.GetMessageType()==0x33){
		AresUtilityClass::DecryptFirstMessage(packet,m_remote_ip,m_fss,m_fsb);
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

		CString log_msg;
		log_msg.Format("Ares Connection:  Received 0x33 init message from %s, compatible",m_str_ip.c_str());
		AresDataCollectorSystemReference ref;
		ref.System()->Log(log_msg);

		//delete []data;
	}*/
	if(packet.GetMessageType()==0x1){
		m_status="Received First Response, Connected";
		AresUtilityClass::DecryptMessage(packet,m_fss,m_fsb);

		m_network_users=*(unsigned int*)(packet.GetDecryptedData()->GetBufferPtr()+2);
		m_network_files=*(unsigned int*)(packet.GetDecryptedData()->GetBufferPtr()+2+4);
		m_network_MBs=*(unsigned int*)(packet.GetDecryptedData()->GetBufferPtr()+2+4+4);
		TRACE("AresDCConnection::ProcessPacket(AresPacket& packet) message 0x1 %u users, %u files, %u MBs\n",m_network_users,m_network_files,m_network_MBs);
		AresDataCollectorSystemReference ref;
		ref.System()->GetHostCache()->AddHosts(mv_hosts);
	}
	else if(packet.GetMessageType()==0x05){
		// Send shares (Message 0x32)
		if(!mb_shares_sent){
			CString log_msg;
			log_msg.Format("Ares Connection:  Received permission to send shares from %s, connection complete",m_str_ip.c_str());
			AresDataCollectorSystemReference ref;
			ref.System()->Log(log_msg);
			SendShares();
		}
	}
	else if(packet.GetMessageType()==0x38){  //new encrypted hello message that brought calamity
		mb_0x38_client=true;
		AresUtilityClass::DecryptFirstMessage(packet,m_remote_ip,m_fss,m_fsb,m_port,true);
		ExtractHosts(packet);

		unsigned char remote_guid[16];
		memcpy(remote_guid,packet.GetDecryptedData()->GetBufferPtr()+2,sizeof(remote_guid));

		// Use the remote guid to send back the message 0x00
		Begin0x38Response(remote_guid);

		//CString log_msg;
		//log_msg.Format("Ares Connection:  Received 0x38 init message from %s, NEW ENCRYPTION",m_str_ip.c_str());
		//AresDataCollectorSystemReference ref;
		//ref.System()->Log(log_msg);
	}
	else if(packet.GetMessageType()==0x25){
		// Send shares (Message 0x32)
		//SendShares();
	}
	else if(packet.GetMessageType()==0x12){
		ProcessSearchResults(&packet);
	}
	else{
		//unknown, unexpected, or irrelevant packet
	}
}

void AresDCConnection::ExtractHosts(AresPacket& packet)
{
	int host_data_length=packet.GetLength()-16-3-2;
	int host_count=host_data_length/6;
	TRACE("AresDCConnection::ExtractHosts() Extraction %d hosts.\n",host_count);
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

LinkedList* AresDCConnection::GetEventList(void)
{
	return &m_event_list;
}

void AresDCConnection::SendMessage0x00(byte* remote_guid)
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

	sprintf(m_user_name,"anon_%x%x%x%03x",fd,sd,td,rand() % 0xFFF);
	
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

	AresDataCollectorSystemReference ref;
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

	//copy the unencrypted data into the encrypted buffer
	p_encrypted_data->WriteBytes(p_unencrypted_data->GetBufferPtr(),p_unencrypted_data->GetLength());

	//encrypt the data in the encrypted buffer
	AresUtilityClass::EncryptMessage(p_encrypted_data->GetMutableBufferPtr()+2,p_encrypted_data->GetLength()-2,piss_encryption_short);

	//create an ares packet so we can record this event for debugging purposes
	AresPacket *new_packet=new AresPacket();
	new_packet->SetType(0x00); //message 00
	new_packet->SetEncryptedData(p_encrypted_data);
	new_packet->SetDecryptedData(p_unencrypted_data);

	//and finally send our message
	if(mp_ps_con){
		new_packet->SendData(mp_ps_con);
		RecordSendEvent(new_packet);
	}
	else{
		delete new_packet;
	}
	in_function=false;
}

void AresDCConnection::OnClose(CAsyncSocket* src){
	//TRACE("AresDCConnection::OnClose() %s:%u\n",m_str_ip.c_str(),m_port);
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

void AresDCConnection::OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection){

}

void AresDCConnection::OnSend(CAsyncSocket* src){
	//TRACE("AresDCConnection::OnSend() %s\n",m_str_ip.c_str());
}

void AresDCConnection::OnReceive(CAsyncSocket* src,byte *data,UINT length){

	if(length>0)
		m_last_receive.Refresh();

	//TRACE("AresDCConnection::OnReceive() %s received %d bytes of data\n",m_str_ip.c_str(),length);
	m_receive_buffer.WriteBytes(data,length);

	AresPacket *packet=new AresPacket();
	if(packet->Read(&m_receive_buffer)){
		ProcessPacket(*packet);
		RecordReceiveEvent(packet);
	}
	else delete packet;
}

void AresDCConnection::OnConnect(CAsyncSocket* src){
	//TRACE("AresDCConnection::OnConnect %s:%u CONNECTION SUCCESSFUL\n",m_str_ip.c_str(),m_port);
	//m_create_time=CTime::GetCurrentTime();
	AresDataCollectorSystemReference ref;
	//CString log_msg;
	//log_msg.Format("Ares Connection:  OnConnect() %s",m_str_ip.c_str());
	//ref.System()->Log(log_msg);

	unsigned char data[]={0x03,0x00,0x5a,0x04,0x03,0x05};
	

	if(mp_ps_con)
		mp_ps_con->Send(data,sizeof(data));
	//ref.System()->GetTCP()->SendData(m_con,data,sizeof(data)); //send the initialization data
	mb_has_connected=true;
	mb_connected=true;
	m_status="Connected - sent init packet";

	if(mb_connecting){
		ref.System()->UpdateConnecting(-1);
		mb_connecting=false;
	}
}

void AresDCConnection::OnConnectFailed(CAsyncSocket* src){
//	TRACE("AresDCConnection::OnConnectFailed() %s\n",m_str_ip.c_str());
	mb_connected=false;
	m_status="Could Not Connect";
	Kill();
}

void AresDCConnection::CleanUp(void)
{
	if(mp_ps_con){
		//mp_ps_con->
		
		
		if(mp_ps_con->m_hSocket!=INVALID_SOCKET){
			SOCKET s=mp_ps_con->Detach();
			closesocket(s);
		}

		//mp_ps_con->Close();
		//mp_ps_con->ShutDown
		delete mp_ps_con;
	}
	mp_ps_con=NULL;

	if(mb_connecting){
		AresDataCollectorSystemReference ref;
		ref.System()->UpdateConnecting(-1);
		mb_connecting=false;
	}

	mb_clean=true;
	//_ASSERTE( _CrtCheckMemory( ) );
}

void AresDCConnection::SendShares(void)
{
	//m_last_file_upload=CTime::GetCurrentTime();
	mb_shares_sent=true;
	m_num_shares=0;

	AresDataCollectorSystemReference ref;
	Vector v_file_group;

	Vector *v_tmp=new Vector();
	srand(GetTickCount());
	v_tmp->Add(new AresFile(NULL,"mdtest.mp3","mdtest","mdtest.mp3","mdtest",665555,665,false,false));
	v_file_group.Add(v_tmp);

	//ref.System()->GetSomeSpoofs(v_file_group);
	if(v_file_group.Size()==0)
		return;

	Vector *v_spoofs=(Vector*)v_file_group.Get(0);

	

	
	CString tmp;

//	mv_files_shared.Clear();
//	mv_files_shared.Add(v_spoofs);//Copy(&v_spoofs);
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
//		m_last_file_upload=CTime::GetCurrentTime();
		SendShares(*(Vector*)v_shares_to_upload.Get(0));
		v_shares_to_upload.Remove(0);
	}

	tmp.Format("Uploading file list: %d files total",v_spoofs->Size());
	m_status=(LPCSTR)tmp;

	//set the hosts fail count to 0 to reset it as a successful host so it doesn't get purged
	AresHost* host=(AresHost*)mv_host.Get(0);
	host->m_fail_count=0;
}

//0x1e
void AresDCConnection::SendPing(void)
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

	if(mp_ps_con){
		new_packet->SendData(mp_ps_con);
		RecordSendEvent(new_packet);
	}
	else{
		delete new_packet;
	}
}

//0x22
void AresDCConnection::SendName(void)
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
	p_unencrypted_data->WriteString(m_user_name);

	//copy the unencrypted data into the encrypted buffer
	p_encrypted_data->WriteBytes(p_unencrypted_data->GetBufferPtr(),p_unencrypted_data->GetLength());

	//encrypt the data in the encrypted buffer
	AresUtilityClass::EncryptMessage(p_encrypted_data->GetMutableBufferPtr()+2,p_encrypted_data->GetLength()-2,piss_encryption_short);

	//create an ares packet so we can record this event for debugging purposes
	AresPacket *new_packet=new AresPacket();
	new_packet->SetType(0x22); //name
	new_packet->SetEncryptedData(p_encrypted_data);
	new_packet->SetDecryptedData(p_unencrypted_data);

	if(mp_ps_con){
		new_packet->SendData(mp_ps_con);
		RecordSendEvent(new_packet);
	}
	else{
		delete new_packet;
	}
}

void AresDCConnection::RecordReceiveEvent(AresPacket* packet)
{
	CSingleLock lock(&m_event_list.m_list_lock,TRUE);
	AresDataCollectorSystemReference ref;
	if(ref.System()->GetEventCacheTime()>0)
		m_event_list.Add(packet);
	else delete packet;
}

void AresDCConnection::RecordSendEvent(AresPacket* packet)
{
	CSingleLock lock(&m_event_list.m_list_lock,TRUE);
	AresDataCollectorSystemReference ref;
	if(ref.System()->GetEventCacheTime()>0)
		m_event_list.Add(packet);
	else delete packet;
}

//call to remove events that have expired past the cache time
void AresDCConnection::PurgeEvents(void)
{
	if(m_event_list.Size()==0)
		return;
	m_event_list.StartIteration();
	AresDataCollectorSystemReference ref;
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

bool AresDCConnection::IsSharingFiles(void)
{
	return mb_shares_sent;
}

int AresDCConnection::SendShares(Vector& v_files)
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

		output.WriteByte(0x01);  //type audio

		output.WriteDWord(the_file->m_size);  //file size

		//Sha-1
		output.WriteBytes(the_file->m_hash,20);

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

		output.WriteByte(0x04);  //bitrate again
		output.WriteWord(192);  

		output.WriteDWord(the_file->m_media_length);  //media length again
		
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

	if(mp_ps_con)
		mp_ps_con->Send(comp_buf,3+comp_len);
	
	AresPacket *new_packet=new AresPacket();
	new_packet->SetType(0x32); //message 32 - send shares
	RecordSendEvent(new_packet);

	delete [] comp_buf;

	return (int)v_files.Size();
}

void AresDCConnection::Search(const char* project,vector <string> &keywords)
{
	AresDataCollectorSystemReference ref;
	CString log_msg;
	log_msg.Format("Ares Connection:  %s SEARCH SUBMITTED project '%s'",m_str_ip.c_str(),project);
	ref.System()->Log(log_msg);

	if(keywords.size()<1)
		return;


	static bool in_function=false;

	ASSERT(!in_function);

	in_function=true;

	Buffer2000 *p_unencrypted_data=new Buffer2000();
	Buffer2000 *p_encrypted_data=new Buffer2000();

	unsigned char encryption_byte=AresUtilityClass::CalcStateByte(0xFE,m_state)+1;
	unsigned short int encryption_short=AresUtilityClass::GetConst591da8(m_fsb*2);//*((unsigned short int *)&const_591da8[m_fsb*2]);
	unsigned short int piss_encryption_short=AresUtilityClass::FourPissAlgorithm(m_fss,encryption_byte,encryption_short);
	unsigned char check_byte=AresUtilityClass::CalcStateByte(0xFA,m_state)+1;	

	p_unencrypted_data->WriteByte(encryption_byte);  //set to encryption byte later
	p_unencrypted_data->WriteByte(check_byte);  //set to encryption byte later

	p_unencrypted_data->WriteDWord(0x00020f64);

	// Add keywords
	for(int i=0;i<(int)keywords.size();i++)
	{
		p_unencrypted_data->WriteByte(0x14);
		p_unencrypted_data->WriteByte((byte)keywords[i].size());
		p_unencrypted_data->WriteWord(AresUtilityClass::CalculateTwoBytesFromBuffer((unsigned char *)keywords[i].c_str(),(UINT)keywords[i].size()));
		p_unencrypted_data->WriteString(keywords[i].c_str());
	}

	//copy the unencrypted data into the encrypted buffer
	p_encrypted_data->WriteBytes(p_unencrypted_data->GetBufferPtr(),p_unencrypted_data->GetLength());

	//encrypt the data in the encrypted buffer
	AresUtilityClass::EncryptMessage(p_encrypted_data->GetMutableBufferPtr()+2,p_encrypted_data->GetLength()-2,piss_encryption_short);

	//create an ares packet so we can record this event for debugging purposes
	AresPacket *new_packet=new AresPacket();
	new_packet->SetType(0x09); //message 09 - query
	new_packet->SetEncryptedData(p_encrypted_data);
	new_packet->SetDecryptedData(p_unencrypted_data);

	//and finally send our message
	if(mp_ps_con){
		new_packet->SendData(mp_ps_con);
		RecordSendEvent(new_packet);
	}
	else{
		delete new_packet;
	}
	in_function=false;

/*
	// Calculate buffer length
	unsigned int buf_len=0;
	buf_len+=sizeof(AresHeader)+2;
	buf_len+=sizeof(unsigned int);		// unknown DWORD
	buf_len+=4*keywords.size();			// headers for the keywords
	for(int i=0;i<keywords.size();i++)
	{
		buf_len+=keywords[i].size();
	}

	// Create buffer
	unsigned char *buf=new unsigned char[buf_len];
	memset(buf,0,buf_len);

	unsigned char *ptr=buf;

	// Set header
	AresHeader *hdr=(AresHeader *)ptr;
	hdr->Op(0x09);
	hdr->Len(buf_len-sizeof(AresHeader));
	ptr+=sizeof(AresHeader);

	ptr+=2;	// for encryption bytes

	// Add unknown dword
	*((unsigned int *)ptr)=0x00020f64;
	ptr+=sizeof(unsigned int);

	// Add keywords
	for(i=0;i<keywords.size();i++)
	{
		*ptr++=0x14;	// "everything" keyword
		*ptr++=keywords[i].size();
		*((unsigned short int *)ptr)=AresUtilityClass::CalculateTwoBytesFromBuffer((unsigned char *)keywords[i].c_str(),keywords[i].size());
		ptr+=sizeof(unsigned short int);
		memcpy(ptr,keywords[i].c_str(),keywords[i].size());
		ptr+=keywords[i].size();
	}

	// Calculate the encryption constants
	unsigned char encryption_byte=AresUtilityClass::CalcStateByte(0xFE,m_state)+1;
	unsigned short int encryption_short=*((unsigned short int *)&const_591da8[m_fsb*2]);
	unsigned short int piss_encryption_short=AresUtilityClass::FourPissAlgorithm(m_fss,encryption_byte,encryption_short);

	// Encrypt 0x09 message
	EncryptMessage(&buf[5],buf_len-5,piss_encryption_short);

	unsigned char check_byte=AresUtilityClass::CalcStateByte(0xFA,m_state)+1;	

	// set buf[3] (encryption byte) and buf[4] (check byte)
	buf[3]=encryption_byte;
	buf[4]=check_byte;

	// Send the message

	char msg[1024];
	strcpy(msg,"Sending Op 0x09 Query Connected Socket : \"");
	for(i=0;i<keywords.size();i++)
	{
		strcat(msg,keywords[i].c_str());
		if(i+1 < keywords.size())
		{
			strcat(msg," ");
		}
	}
	strcat(msg,"\"");
	p_parent->Log(msg,this);

	SendSocketData(buf,buf_len);

	delete [] buf;
*/

}

void AresDCConnection::ProcessSearchResults(AresPacket* packet)
{
	// Make a copy of the message and decrypt it
	AresUtilityClass::DecryptMessage(*packet,m_fss,m_fsb);//DecryptMessage(buf,len);

	//TRACE("Search Response %s:\n",m_str_ip.c_str());
	//for(int i=0;i<packet->GetLength();i++){
	//	byte b=*(byte*)(packet->GetDecryptedData()->GetBufferPtr()+i);
	//	if(b!=0)
	//		TRACE("%d 0x%.2X '%c' \n",i,b,b);
	//	else
	//		TRACE("%d 0x%.2X NULL \n",i,b,b);
	//}

	Buffer2000 *data=packet->GetDecryptedData();

	string file_name;
	string artist;
	string genre;
	string album;
	string title;
	string user_name;
	string comments;
	byte file_hash[20];
	UINT size=0;
	byte media_type=0;
	string year;
	string extension;
	unsigned short bitrate=0;
	UINT media_length=0;

	byte *ptr=data->GetMutableBufferPtr();
	byte *end=data->GetMutableBufferPtr()+data->GetLength();

	byte ip1=ptr[11];
	byte ip2=ptr[12];
	byte ip3=ptr[13];
	byte ip4=ptr[14];

	unsigned short port=*(unsigned short*)(ptr+15);
	CString sip;
	sip.Format("%d.%d.%d.%d",ip1,ip2,ip3,ip4);



	//read in user name
	ptr+=18;
	while(*ptr!=0){
		user_name+=*(char*)ptr;
		ptr++;
	}

	ptr+=6; 

	media_type=*ptr;  //type of file, audio, video
	ptr++;

	size=*(UINT*)ptr;  //size of file
	ptr+=4;

	memcpy(file_hash,ptr,20);  //file hash
	ptr+=20; 

	while(*ptr!=0){
		extension+=*(char*)ptr;
		ptr++;
	}
	ptr++;  //skip past null char

	while(ptr<end){
		byte code=*ptr;
		ptr++;
		if(code==0x01){ //title
			if(title.size()==0){
				while(*ptr!=0 && ptr<end){
					title+=*(char*)ptr;
					ptr++;
				}
			}
			else{
				while(*ptr!=0 && ptr<end){  //genre
					genre+=*(char*)ptr;
					ptr++;
				}
			}
			ptr++;
		}
		else if(code==0x02 && artist.size()==0){ //artist
			while(*ptr!=0 && ptr<end){
				artist+=*(char*)ptr;
				ptr++;
			}
			ptr++;
		}
		else if(code==0x03){ //album
			bool b_comments=false;
			if(media_type==5){
				if(genre.size()==0){  //if its a video we need to treat 0x03 like genre instead of album.
					while(*ptr!=0 && ptr<end){
						genre+=*(char*)ptr;
						ptr++;
					}
				}
				else 
					b_comments=true;  //it is comments and not genre
			}
			else{
				if(album.size()==0){
					while(*ptr!=0 && ptr<end){
						album+=*(char*)ptr;
						ptr++;
					}
				}
				else
					b_comments=true;  //it is comments and not album
			}
			if(b_comments){
				while(*ptr!=0 && ptr<end){
					comments+=*(char*)ptr;
					ptr++;
				}
			}
			ptr++;
		}
		else if(code==0x04){ //bitrate, medialength
			if(bitrate==0){
				bitrate=*(unsigned short*)ptr;
				ptr+=2;
				media_length=*(UINT*)ptr;
				ptr+=4;

				if(media_type==5){ //special thing for movies
					media_length=*(WORD*)(ptr-3);
				}
			}
			else{ //seems to be a language descriptor
				while(*ptr!=0 && ptr<end){
					ptr++;
				}
				ptr++;
			}
		}
		else if(code==0x06){  //year
			while(*ptr!=0 && ptr<end){
				year+=*(char*)ptr;
				ptr++;
			}
			ptr++;
		}
		else if(code==0x10){
			while(*ptr!=0 && ptr<end){
				file_name+=*(char*)ptr;
				ptr++;
			}
			ptr++;			
		}	
		else{  //handle unknown meta tags, this should work but might not if there is something we haven't seen yet.
			while(*ptr!=0 && ptr<end){
				ptr++;
			}
			ptr++;	
		}
	}

	TRACE("Search Result: Username = %s\n",user_name.c_str());

	//CHECK TO SEE IF THIS IS OUR USERNAME
	int val_digit=-1;
	int num_chars=0;

	bool b_is_media_defender=false;

	
	if(port>=30500 && port<=30530)
		b_is_media_defender=true;

	/*
	bool b_go=true;
	const char* the_name=user_name.c_str();
	for(int i=(int)user_name.size()-1;i>=0 && b_go;i--){
		char ch=user_name[i];
		if(ch!='@')
			continue;
		else{ //we have found the '@', previous digit should be number of characters in name if this is us
			b_go=false;
			if(i==0){
				break;  //can't possibly be us, empty user name
			}
			ch=user_name[i-1];  //get char before '@'
			if(ch>128 || !isdigit(ch))  //is the character a digit?
				break;

			//capture the integer that this character before the '@' represents.  it will be the number of characters in the name
			char num[2];
			num[0]=ch;
			num[1]=(char)0;
			val_digit=atoi(num);
			

			for(int j=i-2;j>=0;j--){  //stepping backwards, find the first non-digit.
				ch=user_name[j];

				if(ch>128 || !isdigit(ch)){  //is this a digit?
					//it is not a digit

					int num_chars=j+1;
					if(val_digit==num_chars && num_chars!=0)
						b_is_media_defender=true;
					break;
				}
			}
		}
	}*/

	/*
	for(int i=0;i<(int)user_name.size();i++){
		char ch=user_name[i];
		if(ch<128 && ch>0 && isdigit(ch)){  //when we see a digit, keep assigning our val to it till we stop seeing digits (to parse the last digit)
			char num[2];
			num[0]=ch;
			num[1]=(char)0;
			val_digit=atoi(num);
			b_in_digits=true;
		}
		else if(b_in_digits)  //when we move past the last digit stop
			break;
		else
			num_chars++;
	}*/

	if(b_is_media_defender){
		TRACE("Search Result: Username = %s FILTERED\n",user_name.c_str());
		return;
	}


	CSingleLock lock(&m_search_lock,TRUE);
	if(!AresUtilityClass::IsHashFakeByByte(file_hash) && mv_search_project.Size()>0){
		AresDCProjectInfo *proj=(AresDCProjectInfo*)mv_search_project.Get(0);
		AresSearchResult *ns=new AresSearchResult(file_hash,artist.c_str(),title.c_str(),year.c_str(),genre.c_str(),album.c_str(),comments.c_str(),file_name.c_str(),m_int_ip,size,bitrate,media_length,media_type,sip,port,user_name.c_str());
		proj->AddSearchResult(ns);
	}

	/****** SAMPLE SEARCH RESULT
0 0x09 	 
1 0x12  
2 0x00 
3 0x02  
4 0x00 
5 0x98 ˜ 
6 0x07  
7 0x14  
8 0x78 x 
9 0x6D m 
10 0xAA ª 
11 0x18  
12 0xBA º 
13 0xDB Û 
14 0x65 e 
15 0x2E . 
16 0x21 ! 
17 0x61 a 
18 0x61 a  //USERNAME
19 0x6E n  //USERNAME
20 0x6F o  //USERNAME
21 0x6E n  //USERNAME
22 0x5F _  //USERNAME
23 0x31 1  //USERNAME
24 0x38 8  //USERNAME
25 0x62 b  //USERNAME
26 0x61 a  //USERNAME
27 0x64 d  //USERNAME
28 0x62 b  //USERNAME
29 0x36 6  //USERNAME
30 0x35 5  //USERNAME
31 0x40 @  //USERNAME
32 0x41 A  //USERNAME
33 0x72 r  //USERNAME
34 0x65 e  //USERNAME
35 0x73 s  //USERNAME
36 0x00 
37 0x00 
38 0x00 
39 0x00 
40 0x04    ??WHAT IS THIS
41 0x00 
42 0x01    //TYPE AUDIO  //audio=1, software=3, video=5
43 0x74 t   //SIZE
44 0x56 V   //SIZE
45 0x7F    //SIZE
46 0x00     //SIZE
47 0xFE þ   //FILEHASH
48 0xA0     //FILEHASH
49 0xF7 ÷   //FILEHASH
50 0x20     //FILEHASH
51 0x1C    //FILEHASH
52 0xD1 Ñ   //FILEHASH
53 0x59 Y   //FILEHASH
54 0x66 f   //FILEHASH
55 0xC6 Æ   //FILEHASH
56 0xE6 æ   //FILEHASH
57 0x05    //FILEHASH
58 0x1D    //FILEHASH
59 0xEA ê   //FILEHASH
60 0xAF ¯   //FILEHASH
61 0xB8 ¸   //FILEHASH
62 0xED í   //FILEHASH
63 0x18    //FILEHASH
64 0xE8 è   //FILEHASH
65 0x6C l   //FILEHASH
66 0x24 $   //FILEHASH
67 0x2E .   //EXTENSION
68 0x6D m   //EXTENSION
69 0x70 p   //EXTENSION
70 0x33 3   //EXTENSION
71 0x00   --NULL
72 0x01  //TITLE
73 0x42 B //TITLE
74 0x72 r //TITLE
75 0x65 e //TITLE
76 0x61 a //TITLE
77 0x74 t //TITLE
78 0x68 h //TITLE
79 0x65 e //TITLE
80 0x00   --NULL
81 0x02  //ARTIST
82 0x46 F //ARTIST
83 0x61 a //ARTIST
84 0x62 b //ARTIST
85 0x75 u //ARTIST
86 0x6C l //ARTIST
87 0x6F o //ARTIST
88 0x75 u //ARTIST
89 0x73 s //ARTIST
90 0x00   --NULL
91 0x03  //ALBUM
92 0x45 E //ALBUM
93 0x6D m //ALBUM
94 0x69 i //ALBUM
95 0x6E n //ALBUM
96 0x65 e //ALBUM
97 0x6D m //ALBUM
98 0x00   --NULL
99 0x04   //BITRATE
100 0xD0 Ð //BITRATE  208 Kb
101 0x00   //BITRATE  208 Kb
102 0x41 A //MEDIA LENGTH 
103 0x01  //MEDIA LENGTH 321 seconds
104 0x00   //MEDIA LENGTH 321 seconds
105 0x00   //MEDIA LENGTH 321 seconds
106 0x01   //GENRE
107 0x48 H  //GENRE
108 0x69 i  //GENRE
109 0x70 p  //GENRE
110 0x20    //GENRE
111 0x48 H  //GENRE
112 0x6F o  //GENRE
113 0x70 p  //GENRE
114 0x00   -NULL
115 0x06  //YEAR
116 0x32 2 //YEAR
117 0x30 0 //YEAR
118 0x30 0 //YEAR
119 0x34 4 //YEAR
120 0x00   -NULL
121 0x03  //COMMENTS
122 0x68 h //COMMENTS
123 0x6F o //COMMENTS
124 0x74 t //COMMENTS
125 0x00   - NULL
126 0x05  - 0x05 SOMETHING DUNNO WHAT IT IS THOUGH MAYBE URL
127 0x2D -   --??  
128 0x2D -   --??
129 0x00   - NULL
130 0x10  //FILENAME
131 0x66 f //FILENAME
132 0x61 a //FILENAME
133 0x62 b //FILENAME
134 0x75 u //FILENAME
135 0x6C l //FILENAME
136 0x6F o //FILENAME
137 0x75 u //FILENAME
138 0x73 s //FILENAME
139 0x20   //FILENAME
140 0x2D - //FILENAME
141 0x20   //FILENAME
142 0x62 b //FILENAME
143 0x72 r //FILENAME
144 0x65 e //FILENAME
145 0x61 a //FILENAME
146 0x74 t //FILENAME
147 0x68 h //FILENAME
148 0x65 e //FILENAME
149 0x33 3 //FILENAME
150 0x32 2 //FILENAME
151 0x36 6 //FILENAME
152 0x2E . //FILENAME
153 0x6D m //FILENAME
154 0x70 p //FILENAME
155 0x33 3 //FILENAME
156 0x00    --null char terminated

*//////////////  END SAMPLE SEARCH RESULT ///////////////////////

}

void AresDCConnection::SearchProject(AresDCProjectInfo* project)
{
	CSingleLock lock(&m_search_lock,TRUE);

	if(mb_searched && m_last_search.HasTimedOut(60*5)){  //let the search run for 5 minutes and then kill the connection
		m_status="Finished Search, EOL";
		Kill();
	}

	if(!mb_searched){
		mv_search_project.Clear();
		mv_search_project.Add(project);
	}
}

void AresDCConnection::Begin0x38Response(byte* remote_guid)
{
	ProcessorJob *new_job=new ProcessorJob(remote_guid);
	mv_processor_job.Add(new_job);
	AresDataCollectorSystemReference ref;
	ref.System()->AddProcessorJob(new_job);  //let the system begin crunching on this one
}

bool AresDCConnection::HaveSharesBeenSent(void)
{
	return mb_shares_sent;
}
