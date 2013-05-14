#include "StdAfx.h"
#include "aresinterdictionconnection.h"
#include "AresProtectionSystem.h"
#include "AresUtilityClass.h"

AresInterdictionConnection::AresInterdictionConnection(InterdictionTarget* target)
{
	//unsigned int ip1,ip2,ip3,ip4;
	//sscanf(host_ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);

	if(target->GetRefCount()>0){  //its already in a vector so we should be able to add it here since it must be on the heap
		m_host.Add(target);
	}

	m_ip=target->m_ip;
	m_port=target->m_port;

	char buf[64];
	sprintf( buf , "%u.%u.%u.%u" , (m_ip>>0)&0xff , (m_ip>>8)&0xff , (m_ip>>16) & 0xff ,(m_ip>>24) & 0xff );  //little endian
	m_str_ip=buf;
	
	m_status="Connecting...";

	if(stricmp(target->GetStrIP().c_str(),"81.3.87.174")==0){
		TRACE("AresInterdictionConnection() %s\n",m_str_ip.c_str());
	}

	
	mb_connected=false;
	mb_dead=false;
	m_create_time=CTime::GetCurrentTime();
	m_death_time=CTime::GetCurrentTime();

	mb_marked_for_death=false;

	m_status="Waiting";
	m_state=GetTickCount();

	m_transferred=0;

	//mp_ps_con=new PeerSocket();
	//mp_ps_con->Init(m_str_ip.c_str(),m_port,this);
	m_size=target->m_size;

	memcpy(m_hash,target->m_hash,20);

	m_decryption_code=0x0b87;

	AresProtectionSystemReference ref;
	m_con_handle=ref.System()->GetTCPSystem()->OpenConnection(m_str_ip.c_str(),m_port);
	TRACE("Opening interdiction connection %u to %s.\n",m_con_handle,m_str_ip.c_str());
	if(m_con_handle==0)
		Kill();

	//DWORD val=500;;

	//mp_ps_con->SetSockOpt(SO_RCVBUF,&val,sizeof(DWORD));  //small receive buffer to help with
	//AresUtilityClass::

	//for(int j=0;j<20;j++){
	//	char ch1=hash[j*2];
	//	char ch2=hash[j*2+1];
	//	byte val1=AresUtilityClass::ConvertCharToInt(ch1);
	//	byte val2=AresUtilityClass::ConvertCharToInt(ch2);
	//	byte hash_val=((val1&0xf)<<4)|(val2&0xf);
	//	m_hash[j]=hash_val;
	//}				
}

AresInterdictionConnection::~AresInterdictionConnection(void)
{
	CleanUp();
}

void AresInterdictionConnection::OnClose(){
	if(stricmp(m_str_ip.c_str(),"81.3.87.174")==0){
		TRACE("AresInterdictionConnection::OnClose() %s\n",m_str_ip.c_str());
	}
	mb_connected=false;

	CString tmp_str;
	tmp_str.Format("Connection Lost: %u bytes received",m_transferred);
	m_status=(LPCSTR)tmp_str;

	Kill();

	if(m_transferred==0 && m_host.Size()>0){
		InterdictionTarget* h=(InterdictionTarget*)m_host.Get(0);
		h->mb_banned=true;
	}
	else if(m_transferred>0 && m_host.Size()>0){
		InterdictionTarget* h=(InterdictionTarget*)m_host.Get(0);
		h->mb_banned=false;
	}
}

void AresInterdictionConnection::OnReceive(byte *data,UINT length){

	if(length>0)
		m_last_receive.Refresh();

	//TRACE("AresInterdictionConnection::OnReceive() %s received %d bytes of data\n",m_str_ip.c_str(),length);
	//m_receive_buffer.WriteBytes(data,length);
	//ProcessPacket();

	AresProtectionSystemReference ref;
	ref.System()->AddInterdictionTransferAmount(length);
	m_transferred+=length;

	if(m_transferred<270){  //did we recieve a small packet that would indicate we are being queued?
		AresUtilityClass::DecryptFTPacket(data,length,m_decryption_code);
		
		CString val;
		for(int i=0;i<(int)length;i++){
			byte b=*(data+i);
			if(b==0)
				break;  //should be no nulls in this response
			//else if(b=='\n')
			//	TRACE("IC: %d - 0x%X \\n\n",i);
			//else
			//	TRACE("IC: %d - 0x%X %c\n",i,b,b);

			if(b==NULL)
				break;
			val+=(char)b;
		}

		val=val.MakeLower();
		if(val.Find("503 busy")!=-1 && m_host.Size()>0){
			{//find queue position for inspection
				int queue_index=val.Find("position=");
				if(queue_index!=-1){
					int equal_index=val.Find("=",queue_index+1);
					if(equal_index!=-1){
						int comma_index=val.Find(",",equal_index+1);
						CString position;
						if(comma_index!=-1){  //it may or may not have a comma after it
							position=val.Mid(equal_index+1,comma_index-equal_index-1);
						}
						else
							position=val.Mid(equal_index+1);

						int position_val=atoi(position);
						TRACE("Inderdicting %s, queue position=%d\n",m_str_ip.c_str(),position_val);
						//if(min_poll_val>0 && min_poll_val<1200){ //must be legitimate value
						//	ref.System()->AddInterdictionQueueTarget((InterdictionTarget*)m_host.Get(0),min_poll_val);
						//}
					}
				}			
			}
			{//find poll min
				int queue_index=val.Find("pollmin=");
				if(queue_index!=-1){
					int equal_index=val.Find("=",queue_index+1);
					if(equal_index!=-1){
						int comma_index=val.Find(",",equal_index+1);
						CString min_poll;
						if(comma_index!=-1){  //it may or may not have a comma after it
							min_poll=val.Mid(equal_index+1,comma_index-equal_index-1);
						}
						else
							min_poll=val.Mid(equal_index+1);

						int min_poll_val=atoi(min_poll);
						if(min_poll_val>0 && min_poll_val<1200){ //must be legitimate value
							ref.System()->AddInterdictionQueueTarget((InterdictionTarget*)m_host.Get(0),min_poll_val);
						}
					}
				}
			}
		}


	}

	//TRACE("AresInterdictionConnection::OnReceive() %s now has %u bytes stored\n",m_str_ip.c_str(),m_receive_buffer.GetLength());
}

void AresInterdictionConnection::CleanUp(void)
{
	if(m_con_handle!=NULL){
		AresProtectionSystemReference ref;
		ref.System()->GetTCPSystem()->CloseConnection(m_con_handle);
		m_con_handle=NULL;
	}


	if(stricmp(m_str_ip.c_str(),"81.3.87.174")==0){
		TRACE("AresInterdictionConnection::CleanUp() %s\n",m_str_ip.c_str());
	}
//	m_send_buffer.Clear();   //immediately free this ram for other use
	m_receive_buffer.Clear();//immediately free this ram for other use

//	if(mp_ps_con!=NULL)
//		delete mp_ps_con;
//	mp_ps_con=NULL;
}

void AresInterdictionConnection::Update(void)
{
	if(mb_dead)
		return;

	if(m_last_receive.HasTimedOut(90) && !mb_marked_for_death){
		m_status="idle";
		Kill();
	}

	/*
	if(!mb_marked_for_death && m_send_buffer.GetLength()>0)
		SendData();

	if(m_last_send_purge.HasTimedOut(5)){ //once a minute we can clean out our send buffer
		if(m_send_offset>0){
			m_send_buffer.RemoveRange(0,min((int)m_send_buffer.GetLength(),m_send_offset));
			m_send_offset=0;  //refresh
		}
		m_last_send_purge.Refresh();
	}*/

	CTime now=CTime::GetCurrentTime();
	CTimeSpan alive_time=GetAliveTime();
	CTimeSpan dead_time=CTime::GetCurrentTime()-m_death_time;

	if(!mb_marked_for_death && alive_time.GetTotalSeconds()>(60*60*3)){
		m_status="Very Old";
		Kill();
	}

	/*
	if(mb_marked_for_death && !mb_dead){  //if Kill has been called
		int secs=(int)dead_time.GetTotalSeconds();
		if(secs>4){
			mb_dead=true;
			return;  
		}
		return;
	}*/
}

void AresInterdictionConnection::Kill(void)
{
	if(stricmp(m_str_ip.c_str(),"81.3.87.174")==0){
		TRACE("AresInterdictionConnection::Kill() %s\n",m_str_ip.c_str());
	}
	mb_marked_for_death=true;
	mb_dead=true;
	m_death_time=CTime::GetCurrentTime();
	m_status+=" - Destroying";
	CleanUp();

	if(m_transferred==0 && m_host.Size()>0){
		InterdictionTarget* h=(InterdictionTarget*)m_host.Get(0);
		h->m_fail_count++;
	}
	else if(m_transferred>0 && m_host.Size()>0){
		InterdictionTarget* h=(InterdictionTarget*)m_host.Get(0);
		h->m_fail_count=0;  //reset the fail count since we have transferred data and we know this client is good to interdict
	}

	if(m_transferred>500000 && m_host.Size()>0){
		//queue this one to download again so we can keep interdicting
		AresProtectionSystemReference ref;
		ref.System()->AddInterdictionQueueTarget((InterdictionTarget*)m_host.Get(0),1);
	}
}

void AresInterdictionConnection::ProcessPacket(void)
{
//	if(mp_ps_con==NULL)
//		return;


}

void AresInterdictionConnection::SendData(const byte* data,UINT length)
{
	AresProtectionSystemReference ref;
	ref.System()->GetTCPSystem()->SendData(m_con_handle,data,length);
	/*
	if(mp_ps_con==NULL)
		return;

	if(m_send_offset>=(int)m_send_buffer.GetLength())
		return;

	int send_stat=mp_ps_con->Send(m_send_buffer.GetBufferPtr()+m_send_offset,m_send_buffer.GetLength()-m_send_offset);  //TYDEBUG
	//TRACE("ClientConnection::CheckSendBuffer() %s sent %d bytes.\n",m_ip.c_str(),send_stat);
	if(SOCKET_ERROR==send_stat){
	//	TRACE("ClientConnection::CheckSendBuffer() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
		//mb_abort=true;
		return;
	}
	else if(send_stat>0 && send_stat<20000000){
		m_send_offset+=send_stat;
		//m_send_buffer.RemoveRange(0,min((int)m_send_buffer.GetLength(),(int)send_stat));
		//SetCache(data);  //reset the cache to our remaining data set
	}*/
}

CString AresInterdictionConnection::GetFormattedAgeString(void)
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
void AresInterdictionConnection::MakeFileRequest(void)
{
	TRACE("AresInterdictionConnection::MakeFileRequest() %s\n",m_str_ip.c_str());
	Buffer2000 request;

	request.WriteByte(0xc2); //length of packet i think
	request.WriteByte(0x31);  //????
	request.WriteByte(0x0d);  //numstrcats
	
	//garbage data
	request.WriteByte(0x86);
	request.WriteByte(0xc1);
	request.WriteByte(0x71);
	request.WriteByte(0x4d);
	request.WriteByte(0xd3);
	request.WriteByte(0x38);
	request.WriteByte(0xa9);
	request.WriteByte(0x89);
	request.WriteByte(0xca);
	request.WriteByte(0x0a);
	request.WriteByte(0xae);
	request.WriteByte(0x08);
	request.WriteByte(0x39);
	//end garbage data

	int second_len_offset=request.GetLength();

	request.WriteWord(0xaf);  //length??

	request.WriteByte(0x01);  //always the same??
	request.WriteByte(0x03);
	request.WriteByte(0x00);
	request.WriteByte(0x32);
	request.WriteByte(0x01);

	
	request.WriteWord(m_decryption_code);
	//request.WriteByte(0x87);  //encryption short
	//request.WriteByte(0x0b);

	request.WriteWord(0x14);  //hash length
	request.WriteByte(0x01);  //opcode for hash
	request.WriteBytes(m_hash,20);  //write hash

	//fill in user name
	AresProtectionSystemReference ref;
	string my_user_name=ref.System()->GetUserName(30500+(rand()%30));
	request.WriteWord((WORD)my_user_name.size());
	request.WriteByte(0x02);
	request.WriteString(my_user_name.c_str());

	//23 piss encoded bytes
	request.WriteWord(0x17);
	request.WriteByte(0x06);
	request.WriteByte(0xb2);
	request.WriteByte(0x05);
	request.WriteByte(0x01);
	request.WriteByte(0xba);
	request.WriteByte(0x76);
	request.WriteByte(0xf0);
	request.WriteByte(0x01);
	request.WriteByte(0x55);
	request.WriteByte(0x28);
	request.WriteByte(0xc0);
	request.WriteByte(0xa3);
	request.WriteByte(0x94);
	request.WriteByte(0x09);
	request.WriteByte(0xb8);
	request.WriteByte(0x75);
	request.WriteByte(0x9e);
	request.WriteByte(0xa5);
	request.WriteByte(0x06);
	request.WriteByte(0x4e);
	request.WriteByte(0x60);
	request.WriteByte(0x8a);
	request.WriteByte(0x51);
	request.WriteByte(0xba);

	
	//32 wierd bytes
	request.WriteWord(0x20);
	request.WriteByte(0x0a);
	request.WriteByte(0x37);
	request.WriteByte(0xe3);
	request.WriteByte(0x02);
	request.WriteByte(0xa5);
	request.WriteByte(0x56);
	request.WriteByte(0x10);
	request.WriteByte(0xe7);
	request.WriteByte(0x69);
	request.WriteByte(0x41);
	request.WriteByte(0xe5);
	request.WriteByte(0x57);
	request.WriteByte(0x6e);
	request.WriteByte(0xf6);
	request.WriteByte(0x52);
	request.WriteByte(0x7e);  //15
	request.WriteByte(0x1f);
	request.WriteByte(0xe1);
	request.WriteByte(0xb2);
	request.WriteByte(0x95);
	request.WriteByte(0x67);
	request.WriteByte(0xe8);
	request.WriteByte(0xba);
	request.WriteByte(0x23);
	request.WriteByte(0x5d);
	request.WriteByte(0x17);
	request.WriteByte(0x00);
	request.WriteByte(0x81);
	request.WriteByte(0xb5);
	request.WriteByte(0x0f);
	request.WriteByte(0x9b);
	request.WriteByte(0x61);
	request.WriteByte(0x16);
	
	int end=min(m_size,(1<<20));

	request.WriteWord(0x08);  //8 bytes of range data
	request.WriteByte(0x07);  //range of data op_code
	request.WriteDWord(0);  //start of data range
	request.WriteDWord(end);  //end of data range, ask for up to 2 megs of data

	//wierd range repeat using 64 bits
	request.WriteWord(0x10); //length
	request.WriteByte(0x0b);
	request.WriteDWord(0);  //start of data range 
	request.WriteDWord(0);
	request.WriteDWord(end);  //end of data range, ask for up to 2 megs of data
	request.WriteDWord(0);
	
	request.WriteWord(0x0f);
	request.WriteByte(0x09);
	request.WriteString("Ares 1.8.1.2962");

	string my_ip=ref.System()->GetSystemIP();
	unsigned int ip1=0,ip2=0,ip3=0,ip4=0;
	sscanf(my_ip.c_str(),"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);

	request.WriteWord(0x10);  //16 bytes of ultrapeer info
	request.WriteByte(0x03);  //ultrapeer info op_code

	if(rand()&1){
		request.WriteByte(38);  //ultrapeer ip
		request.WriteByte(119);  //ultrapeer ip
		request.WriteByte(66);  //ultrapeer ip
		request.WriteByte(45);  //ultrapeer ip
		request.WriteWord(4669);
	}
	else{  //try other direction just in case
		request.WriteByte(45);  //ultrapeer ip
		request.WriteByte(66);  //ultrapeer ip
		request.WriteByte(119);  //ultrapeer ip
		request.WriteByte(38);  //ultrapeer ip
		request.WriteWord(4669);
	}
	request.WriteByte(0x04);  //ultrapeer port
	request.WriteByte(0xd8);  //ultrapeer port
	request.WriteByte(ip1);  //my ip - what ultrapeer thinks i am
	request.WriteByte(ip2);  //my ip - what ultrapeer thinks i am
	request.WriteByte(ip3);  //my ip - what ultrapeer thinks i am
	request.WriteByte(ip4);  //my ip - what ultrapeer thinks i am
	request.WriteWord(30500);//my port
	request.WriteByte(ip1);  //my ip - what i think i am
	request.WriteByte(ip2);  //my ip - what i think i am
	request.WriteByte(ip3);  //my ip - what i think i am
	request.WriteByte(ip4);  //my ip - what i think i am

	//request.WriteByte(0x01);
	//request.WriteByte(0x00);
	//request.WriteByte(0x0c);
	//request.WriteByte(0x01);

	request.WriteByte(0x00);  //end
	request.WriteByte(0x00);  //end
	request.WriteByte(0x08);  //end

	//change length offsets
	request.WriteByte(request.GetLength(),0);  //appears to be packet length +1???
	request.WriteByte(request.GetLength()-second_len_offset-2,second_len_offset);  //set second length offset, appears to be corrent without any +1 bs

	//now the encryption part
	byte *buf=request.GetMutableBufferPtr();

	//run the first encryption on part of data
	int offset=3+0x0d+2;
	unsigned short int code=0x3faa;
	for(unsigned int i=offset;i<request.GetLength();i++)
	{
		buf[i]^=(code>>8);
		unsigned char c=buf[i];

		code+=c;
		code*=0xd7fb;
		code+=0x3efd;		
	}


	//run the next encryption on the whole piece of data
	//apply this to the entire first message
	code=0x5d1c;
	for(int i=0;i<(int)request.GetLength();i++)
	{
		
		buf[i]^=(code>>8);
		unsigned char c=buf[i];  

		code+=c;
		code*=0x5ca0;
		code+=0x15ec;		
	}


	SendData(request.GetBufferPtr(),request.GetLength());
	//m_send_buffer.WriteBytes(request.GetBufferPtr(),request.GetLength());
	//SendData();
}

void AresInterdictionConnection::OnConnect(){
	if(stricmp(m_str_ip.c_str(),"81.3.87.174")==0){
		TRACE("AresInterdictionConnection::OnConnect() %s\n",m_str_ip.c_str());
	}
	mb_connected=true;
	MakeFileRequest();
}

UINT AresInterdictionConnection::GetTransferred(void)
{
	return m_transferred;
}

UINT AresInterdictionConnection::GetConHandle(void)
{
	return m_con_handle;
}
