#include "StdAfx.h"
#include "aresftconnection.h"
#include"AresUtilityClass.h"
#include "AresProtectionSystem.h"
#include "AresPoolFile.h"

UINT g_id_ft_counter=0;

UINT AresFTConnection::sm_queued=0;

AresFTConnection::AresFTConnection(UINT con_handle,const char* peer,unsigned short port)
{
	TRACE("AresFTConnection() %s:%u\n",peer,port);
	m_con_handle=con_handle;
	m_port=port;
	//ps->SetSocketEventListener(this);
	//mp_ps_con=ps;
	mb_connected=true;
	mb_dead=false;
	m_create_time=CTime::GetCurrentTime();
	m_death_time=CTime::GetCurrentTime();

	mb_marked_for_death=false;

	m_str_ip=peer;
	//m_port=ps->GetPort();
	m_status="Waiting";
	m_transferred=0;
	m_state=GetTickCount();
	m_id=++g_id_ft_counter;
	//DWORD send_buffer_size=200000;

	//ps->SetSockOpt(SO_SNDBUF,&send_buffer_size,sizeof(send_buffer_size));  //set send buffer to only 100k
}

AresFTConnection::~AresFTConnection(void)
{
	TRACE("~AresFTConnection()\n");
	CleanUp();
}

void AresFTConnection::OnClose(){
	TRACE("AresFTConnection::OnClose() %s\n",m_str_ip.c_str());
	mb_connected=false;

	CString tmp_str;
	tmp_str.Format("Connection Lost: %u bytes sent",m_transferred);
	m_status=(LPCSTR)tmp_str;

	Kill();
}

void AresFTConnection::OnSend(){
	TRACE("AresFTConnection::OnSend() %s\n",m_str_ip.c_str());
	//SendData();
}

void AresFTConnection::OnReceive(byte *data,UINT length){

	if(length>0)
		m_last_receive.Refresh();

	TRACE("AresFTConnection::OnReceive() %s received %d bytes of data\n",m_str_ip.c_str(),length);
	m_receive_buffer.WriteBytes(data,length);
	ProcessPacket();

	TRACE("AresFTConnection::OnReceive() %s now has %u bytes stored\n",m_str_ip.c_str(),m_receive_buffer.GetLength());

	/*
	AresPacket *packet=new AresPacket();
	if(packet->Read(&m_receive_buffer)){
		RecordReceiveEvent(packet);
		ProcessPacket(*packet);
	}
	else delete packet;*/
}

void AresFTConnection::CleanUp(void)
{
	if(m_con_handle!=NULL){
		AresProtectionSystemReference ref;
		ref.System()->GetTCPSystem()->CloseConnection(m_con_handle);
		m_con_handle=NULL;
	}

	//m_send_buffer.Clear();   //immediately free this ram for other use
	m_receive_buffer.Clear();//immediately free this ram for other use
	
	CString file_name;
	file_name.Format("c:\\Ares_Cache\\send_%u.dat",m_id);
	DeleteFile(file_name);  //erase the cache from existence
}

void AresFTConnection::Update(void)
{
	if(mb_dead)
		return;


	if(m_last_receive.HasTimedOut(400) && !mb_marked_for_death){
		m_status="idle";
		Kill();
	}

	//if(!mb_marked_for_death && m_send_buffer.GetLength()>0)
	//	SendData();

	/*
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

	if(!mb_marked_for_death && alive_time.GetTotalSeconds()>7200){
		m_status="Very Old";
		Kill();
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
}

void AresFTConnection::Kill(void)
{
	mb_marked_for_death=true;
	m_death_time=CTime::GetCurrentTime();
	m_status+=" - Destroying";
	CleanUp();
}

void AresFTConnection::ProcessPacket(void)
{
//	if(mp_ps_con==NULL)
//		return;

	m_state=GetTickCount();
	// NOTE : For this test case, the file size is 666,666 bytes
	
	// The only message I should receive is the first packet

	// I think it might try to talk all encrypted first, and if that fails, it tries to talk unencrypted

	// FIRST CHECK TO SEE IF THIS MESSAGE IS NOT ENCRYPTED
	int data_len=m_receive_buffer.GetLength();
	byte *data=m_receive_buffer.GetMutableBufferPtr();

	const char* peer_ip=m_str_ip.c_str(); //debug

	if(data_len<30)  //basic access violation protection
		return;

	if(memcmp(data,"GET sha1:",strlen("GET sha1:"))==0)
	{		
		int x=5;  //debug point
		/*
		char hash[40];
		ZeroMemory(hash,sizeof(hash));
		sscanf((char*)data,"GET sha1:%s=",hash);
		// Make a copy of the data
		char *data_copy=new char[data_len+1];	// +1 for NULL
		memset(data_copy,0,data_len+1);
		memcpy(data_copy,data,data_len);

		// Will need to extract the Base-64 encoded hash ...

		char *ptr=strstr(data_copy,"Range: bytes=");

		if(ptr==NULL)
		{
			m_status="Invalid Receive Data";
			Kill();
			return;
		}

		unsigned int start,end;
		sscanf(ptr,"Range: bytes=%u-%u",&start,&end);

		string reply;
		reply="HTTP/1.1 206 OK\r\n";
		reply+="Server: Ares 1.8.1.2945\r\n";
		reply+="X-My-Nick: sexytime\r\n";
		reply+="X-B6MI: h8jP5STyCN7vvT6y\r\n";	// it doesn't seem to care what's in here
		reply+="X-MyLIP: 3EB25D9C\r\n";
		reply+="Connection: Keep-Alive\r\n";
		
		char msg[1024];
		sprintf(msg,"Content-range: bytes=%u-%u/%u\r\n",start,end,end-start+1);	
		reply+=msg;

		reply+="\r\n";

		// Send the message and the data
		//SendSocketData((unsigned char *)reply.c_str(),(unsigned int)reply.size());
		m_send_buffer.WriteBytes((byte*)reply.c_str(),(int)reply.size());
		SendData();

		// Create data and encrypt it
		unsigned char *mp3=new unsigned char[end-start+1];
		memset(mp3,0,end-start+1);

		// And send it
		//SendSocketData(mp3,end-start+1);
		m_send_buffer.WriteBytes(mp3,end-start+1);
		SendData();
		*/
	}
	else	// IT IS ENCRYPTED
	{
		unsigned char *buf=NULL;
		unsigned int len=0;

		// Make a copy of the data
		Buffer2000 data_copy(data,data_len);
		//unsigned char *data_copy=new unsigned char[data_len];
		//memcpy(data_copy,data,data_len);

		buf=data_copy.GetMutableBufferPtr();
		len=data_len;

		// Like the decrypt first message function
		unsigned short int code=0x5d1c;
		for(int i=0;i<(int)data_len;i++)
		{
			unsigned char c=buf[i];
			buf[i]^=(code>>8);

			code+=c;
			code*=0x5ca0;
			code+=0x15ec;		
		}

		unsigned char num_strcats=buf[2];

		// Like the decrypt first message function, but the constants are different

		//access violation protection
		if(data_len<(int)(3+num_strcats+sizeof(unsigned short int)))
			return;
		

		buf=data_copy.GetMutableBufferPtr()+3+num_strcats+sizeof(unsigned short int);
		len=*(unsigned short int *)(data_copy.GetMutableBufferPtr()+3+num_strcats);//&data_copy[3+num_strcats]);
		

		//more access violation protection
		if(data_len<(int)(3+num_strcats+sizeof(unsigned short int)+len))
			return;

		code=0x3faa;
		for(unsigned int i=0;i<len;i++)
		{
			unsigned char c=buf[i];
			buf[i]^=(code>>8);

			code+=c;
			code*=0xd7fb;
			code+=0x3efd;		
		}

		
		if(data_len>2047){  //crash protection in case of bad data
			m_receive_buffer.Clear();
			return;
		}

		// Make a copy so that i can look at it
		unsigned char decomp[2048];
		memset(decomp,0,sizeof(decomp));
		memcpy(decomp,data_copy.GetMutableBufferPtr(),data_len);

		//for(int i=0;i<data_len;i++){
		//	TRACE("%d - 0x%X\n",i,*(byte*)(data_copy.GetBufferPtr()+i));
		//}

		//unsigned short encryption_code=0;
		//parse our message and extract the relevant information

		// Find the encryption short
		code=*(unsigned short int *)(data_copy.GetMutableBufferPtr()+3+num_strcats+2+5);//&data_copy[3+num_strcats+2+5]);

		byte *p_data=(data_copy.GetMutableBufferPtr()+(3+num_strcats+2+5+2));//&data_copy[3+num_strcats+2+5+2];

		//vars to fill in
		byte *p_sha1=NULL;
		UINT start=0;
		UINT end=0;

		int tag_count=0;  //corruption check to make sure we don't get stuck in the next loop

		bool b_respond_hash_set=false;;

		while(tag_count++<200 && (p_data<data_copy.GetMutableBufferPtr()+data_len)){
			unsigned short hdr_len=*(unsigned short*)p_data;
			p_data+=2;
			byte op_code=*p_data;
			p_data+=1;

			if(op_code==0x01){
				p_sha1=p_data;  //mark where the sha1 is located in the buffer
			}
			else if(op_code==0x07){
				start=*(UINT*)p_data;
				end=*(UINT*)(p_data+4);

				if(end<start){
					UINT tmp=end;
					end=start;  //swap
					start=tmp;
				}
			}
			else if(op_code==0xc){
				b_respond_hash_set=true;
			}
				

			p_data+=hdr_len;  //skip the length of this header
		}

		if(p_sha1==NULL || end==0){
			m_receive_buffer.Clear();
			return; //we didn't parse something correctly
		}

		AresProtectionSystemReference ref;
		Vector v_tmp_pool_decoy;
		ref.System()->GetDecoyPool()->GetDecoy(p_sha1,v_tmp_pool_decoy);

		UINT tmp_file_len=0;
		AresPoolFile *apf=NULL;
		if(v_tmp_pool_decoy.Size()>0){
			apf=(AresPoolFile*)v_tmp_pool_decoy.Get(0);
			tmp_file_len=apf->m_size;
		}

		if(tmp_file_len==0)
			tmp_file_len=ref.System()->GetFileLength(p_sha1);

		if(tmp_file_len==0){ //this is a decoy, we will usually ignore this request because we need to be focusing on swarms
			m_status="Decoy - Ignored";
			Kill();
			return;
		}

		if(b_respond_hash_set){
			RespondHashSet(p_sha1,start,end,code);
			return;
		}

		srand(GetTickCount());

		UINT file_len=*(UINT*)(p_sha1+2);

		if((end-start)>2500000)
			end=start+2500000;

		/*
		if(start>((1*(1<<20))+1)){  //greater than 1 megs, lets just cancel the connection to save bandwidth
			m_receive_buffer.Clear();
			return;
		}*/
		
		// Find the start and end values
		//unsigned char *ptr=&data_copy[3+num_strcats+2+5+2+3+20];
		//ptr+=*((unsigned short int *)ptr)+3;	// username and username header

		//ptr+=0x17+3;

		//ptr+=3;									// range header
		//unsigned int start=*((unsigned int *)ptr);
		//byte* start_offset_ptr=ptr;
		//ptr+=sizeof(unsigned int);
		//unsigned int end=*((unsigned int *)ptr);

		m_transferred+=(end-start+1);

		CString tmp_str;
		tmp_str.Format("Encrypted Upload: %u bytes sent total.",m_transferred);
		m_status=(LPCSTR)tmp_str;

		if(tmp_file_len!=0)
			file_len=tmp_file_len;  //its a swarm, lets not use the length encoded into the hash because its not really encoded there for swarms.

		if(end>=file_len)
			end=file_len-1;

		/*
		TRACE("\n\nFTDATA: ");
		for(int buf_index=0;buf_index<data_len;buf_index++){
			TRACE("%x ",data_copy[buf_index]);
		}
		TRACE("\n\n");*/

		string reply;
		reply="HTTP/1.1 206 OK\r\n";
		reply+="Server: Ares 1.8.1.2966\r\n";
		reply+="X-My-Nick: ";
		string my_user_name=ref.System()->GetUserName(m_port);
		reply+=my_user_name.c_str();
		reply+="\r\n";

		reply+="X-B6MI: h8jP5STyCN7vvT6y\r\n";	// it doesn't seem to care what's in here
		reply+="X-MyLIP: 3EB25D9C\r\n";
		reply+="Connection: Keep-Alive\r\n";
		
		char msg[1024];
		sprintf(msg,"Content-range: bytes=%u-%u/%u\r\n",start,end,file_len);	
		reply+=msg;

		reply+="\r\n";

		unsigned int num_state_bytes=AresUtilityClass::CalcStateByte(0xf,m_state);

		unsigned int reply_len=3+num_state_bytes+(unsigned int)reply.size();
		unsigned char *reply_buf=new unsigned char[reply_len];
		memset(reply_buf,0,reply_len);
		
		for(unsigned int i=0;i<3+num_state_bytes;i++)
		{
			if(i==2)
			{
				reply_buf[i]=num_state_bytes;
			}
			else
			{
				reply_buf[i]=AresUtilityClass::CalcStateByte(0xff,m_state);
			}
		}

		memcpy(&reply_buf[3+num_state_bytes],reply.c_str(),reply.size());

		// Encrypt message
		code=AresUtilityClass::EncryptFTPacket(reply_buf,reply_len,code);

		// And send it
		//m_send_buffer.WriteBytes(reply_buf,reply_len);
		SendData(reply_buf,reply_len);
		delete []reply_buf;
		//sm_queued+=reply_len;
		//SendData();
		//SendSocketData(reply_buf,reply_len);

		int req_len=end-start+1;

		// Create data and encrypt it
		unsigned char *mp3=new unsigned char[req_len];
		//memset(mp3,0,end-start+1);

		//fill it in with mp3 noise
		
		int mp3_offset=0;
		Buffer2000* mp3_noise=NULL;
		if(apf==NULL)
			mp3_noise=ref.System()->GetMP3NoiseBuffer();
		else
			mp3_noise=&apf->m_sig_buf2k;

		//append the mp3 noise starting at the correct offset, this is important because we need to match our hash parts no matter what offset they ask for
		int start_offset=start;
		int tmp_len=req_len;
		UINT sig_offset=start_offset%(mp3_noise->GetLength());
		UINT init_copy_len=min((mp3_noise->GetLength()-sig_offset),(UINT)tmp_len);
		memcpy(mp3,mp3_noise->GetMutableBufferPtr()+sig_offset,init_copy_len);
		start_offset+=init_copy_len;
		tmp_len-=init_copy_len;
		UINT cur_buf_pos=init_copy_len;
		while(tmp_len>0){
			memcpy(mp3+cur_buf_pos,mp3_noise->GetMutableBufferPtr(),min(mp3_noise->GetLength(),(UINT)tmp_len));
			cur_buf_pos+=mp3_noise->GetLength();
			tmp_len-=min(mp3_noise->GetLength(),(UINT)tmp_len);
		}


		//while(mp3_offset<req_len){
		//	int copy_len=min((int)mp3_noise->GetLength(),req_len-mp3_offset);
		//	memcpy(mp3+mp3_offset,mp3_noise->GetBufferPtr(),copy_len);
		//	mp3_offset+=mp3_noise->GetLength();
		//}

		code=AresUtilityClass::EncryptFTPacket(mp3,end-start+1,code);

		// And send it
		//SendSocketData(mp3,end-start+1);
		//m_send_buffer.WriteBytes(mp3,end-start+1);
		SendData(mp3,end-start+1);
		//if(p_sha1!=NULL)
		//	ref.System()->BanClient(m_str_ip.c_str(),p_sha1);
		delete []mp3;
		//sm_queued+=(end-start+1);
		//AppendToCache(send_buffer);  //flush all this out to file and begin sending it
		//SendData();
		m_receive_buffer.Clear();
	}
}

void AresFTConnection::SendData(const byte *data, UINT length)
{
	AresProtectionSystemReference ref;
	ref.System()->GetTCPSystem()->SendData(m_con_handle,data,length);

	/*
	if(mp_ps_con==NULL)
		return;

	if(m_send_offset>=(int)m_send_buffer.GetLength())
		return;

	//int send_stat=mp_ps_con->Send(m_send_buffer.GetBufferPtr()+m_send_offset,min(4096,m_send_buffer.GetLength()-m_send_offset));  //TYDEBUG
	int send_stat=mp_ps_con->Send(m_send_buffer.GetBufferPtr()+m_send_offset,m_send_buffer.GetLength()-m_send_offset);  //TYDEBUG
	//TRACE("ClientConnection::CheckSendBuffer() %s sent %d bytes.\n",m_ip.c_str(),send_stat);
	if(SOCKET_ERROR==send_stat){
	//	TRACE("ClientConnection::CheckSendBuffer() had an error %d while sending data to %s.\n",GetLastError(),m_ip.c_str());
		//mb_abort=true;
		return;
	}
	else if(send_stat>0 && send_stat<20000000){
		AresProtectionSystemReference ref;
		ref.System()->AddDataSentEvent(send_stat);
		m_send_offset+=send_stat;
		//m_send_buffer.RemoveRange(0,min((int)m_send_buffer.GetLength(),(int)send_stat));
		//SetCache(data);  //reset the cache to our remaining data set
	}*/
}

CString AresFTConnection::GetFormattedAgeString(void)
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

//call to get whatever data we have cached to send
/*
void AresFTConnection::GetCache(Buffer2000& data)
{
	CString file_name;
	file_name.Format("c:\\Ares_Cache\\send_%u.dat",m_id);

	HANDLE file_handle=CreateFile(file_name, // open file at local_path 
				GENERIC_READ,              // open for reading 
				FILE_SHARE_READ,           // share for reading 
				NULL,                      // no security 
				OPEN_EXISTING,             // always open a file
				FILE_ATTRIBUTE_NORMAL,     // normal file 
				NULL);                     // no attr. template 

	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
		CloseHandle(file_handle);
		return;
	}

	DWORD tmp=0;
	DWORD len=GetFileSize(file_handle,&tmp);
	if(len==0 || len>5000000){
		CloseHandle(file_handle);
		return;
	}

	byte *tmp_buf=new byte[len];
	tmp=0;
	ReadFile(file_handle,tmp_buf,len,&tmp,NULL);  //read in the cache and then store it in the data object
	data.WriteBytes(tmp_buf,len); 
	delete []tmp_buf; 

	CloseHandle(file_handle);
}

void AresFTConnection::AppendToCache(Buffer2000& data)
{
	CString file_name;
	file_name.Format("c:\\Ares_Cache\\send_%u.dat",m_id);

	HANDLE file_handle=CreateFile(file_name, // open file at local_path 
				GENERIC_WRITE,              // open for reading 
				FILE_SHARE_READ,           // share for reading 
				NULL,                      // no security 
				OPEN_ALWAYS,             // always open a file
				FILE_ATTRIBUTE_NORMAL,     // normal file 
				NULL);                     // no attr. template 

	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
		TRACE("AresFTConnection %s: Could not save cache with length %u.\n",m_str_ip.c_str(),data.GetLength());
		CloseHandle(file_handle);
		return;
	}

	DWORD tmp=0;
	DWORD len=GetFileSize(file_handle,&tmp);

	LONG high_move=0;  //if we pass in 0 instead of an actual long address, then low_move gets treated as a signed value and we can't work with sizes between 2 gigs and 4 gigs.
	LONG low_move=len;  
	DWORD dwPtr=SetFilePointer(file_handle,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading

	tmp=0;
	WriteFile(file_handle,data.GetBufferPtr(),data.GetLength(),&tmp,NULL);  //append this data to the end of the file
	CloseHandle(file_handle);

}

void AresFTConnection::SetCache(Buffer2000& data)
{
	CString file_name;
	file_name.Format("c:\\Ares_Cache\\send_%u.dat",m_id);

	HANDLE file_handle=CreateFile(file_name, // open file at local_path 
				GENERIC_WRITE,              // open for reading 
				FILE_SHARE_READ,           // share for reading 
				NULL,                      // no security 
				CREATE_ALWAYS,             // always open a file
				FILE_ATTRIBUTE_NORMAL,     // normal file 
				NULL);                     // no attr. template 

	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
		TRACE("AresFTConnection %s: Could not save cache with length %u.\n",m_str_ip.c_str(),data.GetLength());
		CloseHandle(file_handle);
		return;
	}

	DWORD tmp=0;
	WriteFile(file_handle,data.GetBufferPtr(),data.GetLength(),&tmp,NULL);  //append this data to the end of the file
	CloseHandle(file_handle);
}*/

void AresFTConnection::RespondHashSet(byte* p_hash, UINT start, UINT end, unsigned short code)
{
	AresProtectionSystemReference ref;
	srand(GetTickCount());

	UINT file_len=*(UINT*)(p_hash+2);

	if((end-start)>2500000)
		end=start+2500000;

	CString tmp_str;
	tmp_str.Format("Respond Hash Parts");
	m_status=(LPCSTR)tmp_str;

	Vector v_tmp_pool_decoy;
	ref.System()->GetDecoyPool()->GetDecoy(p_hash,v_tmp_pool_decoy);

	UINT tmp_file_len=0;
	AresPoolFile *apf=NULL;
	if(v_tmp_pool_decoy.Size()>0){
		apf=(AresPoolFile*)v_tmp_pool_decoy.Get(0);
		tmp_file_len=apf->m_size;
	}

	if(tmp_file_len==0)
		tmp_file_len=ref.System()->GetFileLength(p_hash);

	if(tmp_file_len!=0)
		file_len=tmp_file_len;  //its a swarm, lets not use the length encoded into the hash because its not really encoded there for swarms.

	if(end>=file_len)
		end=file_len-1;

	/*
	TRACE("\n\nFTDATA: ");
	for(int buf_index=0;buf_index<data_len;buf_index++){
		TRACE("%x ",data_copy[buf_index]);
	}
	TRACE("\n\n");*/

	int num_chunks=file_len/HASHCHUNKSIZE;
	if((file_len%HASHCHUNKSIZE)!=0)
		num_chunks++;

	byte* part_hashes=new byte[num_chunks*20];

	byte *ptr=part_hashes;
	UINT len=file_len;
	for(int i=0;i<num_chunks;i++){
		if(apf==NULL)
			ref.System()->CalculateHash(i*HASHCHUNKSIZE,min((int)len,HASHCHUNKSIZE),ptr+i*20);
		else{
			if(i<num_chunks-1)
				memcpy(ptr+i*20,apf->m_part_hash,20); //use pool file hash part instead
			else
				memcpy(ptr+i*20,apf->m_last_part_hash,20); //use pool file hash part instead, but use the special last part hash which has a different hash because it has an arbitrary length
		}
		len-=min((int)len,HASHCHUNKSIZE);
	}


	//memcpy(part_hash,ref.System()->Get256KHash(),20);

	

	//}
	/*
	else if(file_len<20000000){  //~20 megs
		hash_chunk_size=(1<<20); //1 meg part size
		memcpy(part_hash,ref.System()->Get1MegHash(),20);
	}
	else if(file_len<100000000){ //~100 megs
		hash_chunk_size=2*(1<<20);
		memcpy(part_hash,ref.System()->Get2MegHash(),20);
	}
	else{  //anything larger gets a 5 meg hash size
		hash_chunk_size=5*(1<<20);
		memcpy(part_hash,ref.System()->Get5MegHash(),20);
	}*/


	//int remainder=file_len;

	//while(remainder>hash_chunk_size)
	//	remainder-=hash_chunk_size;  //size of last chunk so we can request it

	//ref.System()->CalculateHash(remainder,last_hash);

	CString str_hash=AresUtilityClass::EncodeBase16(p_hash,20);
	CString str_hash_chunk_size;
	str_hash_chunk_size.Format("%d",HASHCHUNKSIZE);
	CString str_data_len;
	str_data_len.Format("%d",50+num_chunks*20);

	string reply;
	reply="HTTP/1.1 200 OK\r\n";
	reply+="Server: Ares 1.8.1.2966\r\n";
	reply+="PHashIdx:";  //encode the hash in base 16 ascii format
	reply+=(LPCSTR)str_hash;
	reply+="\r\n";
	reply+=(LPCSTR)str_hash_chunk_size;
	reply+="\r\n";
	reply+="X-B6MI: s7ro+vYc4as+yokh\r\n";	// it doesn't seem to care what's in here
	reply+="X-MyLIP: 42CF1D7E\r\n";
	reply+="Content-Length: ";
	reply+=(LPCSTR)str_data_len;
	reply+="\r\n";		
	reply+="\r\n";

	unsigned int num_state_bytes=AresUtilityClass::CalcStateByte(0xf,m_state);

	unsigned int reply_len=3+num_state_bytes+(unsigned int)reply.size();
	unsigned char *reply_buf=new unsigned char[reply_len];
	memset(reply_buf,0,reply_len);
	
	for(unsigned int i=0;i<3+num_state_bytes;i++)
	{
		if(i==2)
		{
			reply_buf[i]=num_state_bytes;
		}
		else
		{
			reply_buf[i]=AresUtilityClass::CalcStateByte(0xff,m_state);
		}
	}

	memcpy(&reply_buf[3+num_state_bytes],reply.c_str(),reply.size());

	// Encrypt message
	code=AresUtilityClass::EncryptFTPacket(reply_buf,reply_len,code);

	// And send it
	SendData(reply_buf,reply_len);
	//m_send_buffer.WriteBytes(reply_buf,reply_len);
	delete []reply_buf;

	//SendData();

	Buffer2000 hash_set_response;
	hash_set_response.WriteString("__ARESDBP102__");
	hash_set_response.WriteDWord(1);
	hash_set_response.WriteDWord(0);
	hash_set_response.WriteDWord(num_chunks*20);
	hash_set_response.WriteBytes(p_hash,20);
	hash_set_response.WriteDWord(1);

	for(int i=0;i<num_chunks;i++)
		hash_set_response.WriteBytes(part_hashes+i*20,20);

	code=AresUtilityClass::EncryptFTPacket(hash_set_response.GetMutableBufferPtr(),hash_set_response.GetLength(),code);

	// And send it
	//SendSocketData(mp3,end-start+1);
	//m_send_buffer.WriteBytes(hash_set_response.GetMutableBufferPtr(),hash_set_response.GetLength());
	SendData(hash_set_response.GetMutableBufferPtr(),hash_set_response.GetLength());

	//AppendToCache(send_buffer);  //flush all this out to file and begin sending it
	//SendData();
	delete []part_hashes;
	m_receive_buffer.Clear();
}

UINT AresFTConnection::GetConHandle(void)
{
	return m_con_handle;
}
