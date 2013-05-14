#include "StdAfx.h"
#include "udpsearcher.h"
#include "..\AresProtector\AresUtilityClass.h"
#include "..\AresDatacollector\ProcessorJob.h"
#include "AresUDPDataCollectorSystem.h"


UDPSearcher::UDPSearcher(void)
{
	AfxSocketInit();

	BOOL stat=m_rcv_socket.Create(BASEPORT,SOCK_DGRAM,0,NULL);
	if(!stat){
		int error=GetLastError();
		TRACE("Couldn't create UDP Supernode Socket error = %d\n",error);
	}
}

UDPSearcher::~UDPSearcher(void)
{
}

UINT UDPSearcher::Run(void)
{
	srand(GetTickCount());
	while(!this->b_killThread){
		byte buf[512];
		CString addr;
		UINT port=0;
		int nread=m_rcv_socket.ReceiveFrom(buf,512,addr,port);  
		if(nread==SOCKET_ERROR){
			int error=GetLastError();
			Sleep(5);
			continue;
//			TRACE("ED2KAntiLeakServer::Run() failed to receive udp data because %d.\n",error);
		}
		if(nread<5){
			continue;
		}

		if(buf[0]!='A' && buf[1]!='R' && buf[2]!='E'){  //make sure it is an Ares packet
			continue;
		}

		byte op_code=buf[3];



		Vector v_tmp;
		//TYDEBUG
		IsConnectedToSupernode(addr,v_tmp);
			//continue;
		//}


		//if(op_code!=0x22 && op_code!=0x1f && op_code!=0x20 && op_code!=0x1a && op_code!=0x1e && op_code!=0x1c && op_code!=0x1b && op_code!=0x21){
			TRACE("UDP RECEIVE OPCODE 0x%X length %u from %s:%u\n",op_code,nread,addr,port);
		//}

		AresUDPSupernode *host=NULL;
		if(v_tmp.Size()>0){
			host=(AresUDPSupernode*)v_tmp.Get(0);
		}
		
		if(op_code==0x1f && nread>6 && host==NULL){
			unsigned short tmp_port1=*(unsigned short*)(buf+4);
			unsigned short tmp_port2=*(unsigned short*)(buf+6);
			unsigned short tmp_port3=*(unsigned short*)(buf+8);

			//let this host connect to us
			//TYDEBUG - Comment following block out to disable/enable receiving udp connections from other supernodes
			
			
			if(tmp_port1!=4685 && tmp_port1!=BASEPORT){ //do not communicate with our own supernodes
				Vector v_tmp;
				CSingleLock lock(&m_lock,TRUE);

				
				SystemRef ref;
				int index=GetBinFromPeer(addr);
				AresUDPSupernode *nh=new AresUDPSupernode(addr,tmp_port1);
				mv_udp_connections[index].Add(nh);
				nh->mb_accepted=true;
				SendMessage0x20(nh);  

				//ref.System()->SaveHost(addr,tmp_port1);
			}
			
			//TRACE("Adding tmp host %s:%u other ports may be %u, %u\n",addr,tmp_port1,tmp_port2,tmp_port3);
			//AddHost(addr,tmp_port1);  //treat this as a new host, it isn't connected unless it sends something back after this
		}
		if(host!=NULL && op_code==0x20 && nread>=0x80+4){
			CSingleLock lock(&m_processor_job_lock,TRUE);
			byte* eighty=buf+4;
//			memcpy(m_captured_eighty,eighty,0x80);
//			mb_captured_eighty=true;
			host->mv_processor_job.Clear();
			ProcessorJob *new_job=new ProcessorJob(eighty);
			host->mv_processor_job.Add(new_job);
			mv_queued_jobs.Add(host);
			SystemRef ref;
			ref.System()->AddProcessorJob(new_job);

			if(!host->mb_alive)
				m_alive_host_count++;

			if(!host->mb_accepted && !host->mb_alive)
				m_alive_host_count_outbound++;

			host->mb_alive=true;
			host->m_last_response=CTime::GetCurrentTime();

		}
		else if(host!=NULL && op_code==0x22){
			if(!host->mb_alive)
				m_alive_host_count++;

			SystemRef ref;
//			AresHost local_host(ref.System()->GetSystemIP(),4865);
//			TRACE("Message 0x22 Breakdown:  %s:%u - Special Word=%u, Guess:  %u\n",addr,host->GetPort(),*(WORD*)(buf+4),host->m_tmp_word);			
			//for(int i=0;i<nread;i++){
			//	byte b=buf[i];

			//	if(b!=0){
			//		TRACE("%d 0x%.2X '%c' \n",i,b,b);
			//	}
			//	else{
			//	TRACE("%d 0x%.2X NULL \n",i,b,b);
			//	}
			//}

			
			host->mb_alive=true;
			host->m_last_response=CTime::GetCurrentTime();

//			if(host->mb_saveable){
				//this host has connected to us, instead of vice versa.  thus we want to save it to our database of supernodes
				
//				ref.System()->SaveHost(host->GetIP(),host->GetPort());
//				host->mb_saveable=false;
//			}
		}
		else if(host!=NULL && op_code==0x21){
			if(!host->mb_alive)
				m_alive_host_count++;

			
			CSingleLock lock(&m_processor_job_lock,TRUE);
			byte* eighty=buf+4+2+20;

			host->mv_processor_job.Clear();
			ProcessorJob *new_job=new ProcessorJob(eighty);
			host->mv_processor_job.Add(new_job);
			mv_queued_jobs.Add(host);
			SystemRef ref;
			ref.System()->AddProcessorJob(new_job);
			

			//SendMessage0x22(host);

			//we must wait for the processor to do its work before we response to this message with the proper 0x22 op_code
			host->mb_alive=true;
			host->m_last_response=CTime::GetCurrentTime();

//			if(host->mb_saveable){
				//this host has connected to us, instead of vice versa.  thus we want to save it to our database of supernodes
				
//				ref.System()->SaveHost(host->GetIP(),host->GetPort());
//				host->mb_saveable=false;
//			}
		}
		else if(host!=NULL && op_code==0x1c){
			
//			TRACE("PREQUERY HIT:  %s:\n",addr);			
//			for(int i=0;i<nread;i++){
//				byte b=buf[i];
//
//				if(b!=0){
//					TRACE("%d 0x%.2X '%c' \n",i,b,b);
//				}
//				else{
//					TRACE("%d 0x%.2X NULL \n",i,b,b);
//				}
//			}

			if(!host->mb_alive)
				m_alive_host_count++;
			host->mb_alive=true;
			host->m_last_response=CTime::GetCurrentTime();
		}
		else if(host!=NULL && op_code==0x1e && nread==10){
			if(host->m_last_1e_receive!=host->m_create_time){
				CTimeSpan ts=CTime::GetCurrentTime()-host->m_last_1e_receive;
			//	TRACE("Host %s received 0x1E packet %u seconds after last 0x1E packet\n",host->GetIP(),ts.GetTotalSeconds());
			}

			host->m_last_1e_receive=CTime::GetCurrentTime();


			//for(int i=0;i<nread;i++){
			//	byte b=buf[i];
			//	if(b!=0){
			//		TRACE("%d 0x%.2X '%c' \n",i,b,b);
			//	}
			//	else{
			//		TRACE("%d 0x%.2X NULL \n",i,b,b);
			//	}
			//}

			byte ip1=buf[4];
			byte ip2=buf[5];
			byte ip3=buf[6];
			byte ip4=buf[7];
			unsigned short new_port=*(unsigned short*)(buf+8);

			CString new_ip;
			new_ip.Format("%u.%u.%u.%u",ip1,ip2,ip3,ip4);

			//we just received information about a new server, let us try adding this as a host

			if(mb_captured_0x1e)  //send this back in response
				SendMessage0x1E(host);

			memcpy(m_captured_0x1E,buf,10);
			mb_captured_0x1e=true;

			if(!host->mb_alive)
				m_alive_host_count++;
			host->mb_alive=true;
			host->m_last_response=CTime::GetCurrentTime();
		}
		else if(host!=NULL && op_code==0x1b){
			ProcessSearchResult(buf,nread);
			//UINT fake_query_id=*(UINT*)(buf+4);

			CTime now=CTime::GetCurrentTime();
			host->m_last_search_result=now;
			
			//TRACE("QUERY HIT:  %s:\n",addr);
//			for(int i=0;i<nread;i++){
//				byte b=buf[i];
//				if(b!=0){
//					TRACE("%d 0x%.2X '%c' \n",i,b,b);
//				}
//				else{
//					TRACE("%d 0x%.2X NULL \n",i,b,b);
//				}
//			}
		}
		else if(host!=NULL && op_code==0x1a && nread>=8){  //query
			TRACE("Received query from %s\n",host->GetIP());
			if(!host->mb_alive)
				m_alive_host_count++;
			host->mb_alive=true;
			host->m_last_response=CTime::GetCurrentTime();
			host->m_last_search=CTime::GetCurrentTime();

			//TEST ********  FORWARD QUERY TO RANDOM HOST AND LOOK AT RESPONSE

			UINT real_query_id=*(UINT*)(buf+4);

			RespondQuery(real_query_id,host); //immediately send back the 0x1c message
		}
	}
	return 0;
}

bool UDPSearcher::IsConnectedToSupernode(const char* host_ip, Vector& v_tmp)
{
	int index=GetBinFromPeer(host_ip);
	CSingleLock lock(&m_lock,TRUE);
	for(int i=0;i<(int)mv_udp_connections[index].Size();i++){
		AresUDPSupernode *ah=(AresUDPSupernode*)mv_udp_connections[index].Get(i);

		if(stricmp(ah->GetIP(),host_ip)==0){
			v_tmp.Add(ah);
			return true;
		}
	}

	return false;
}


void UDPSearcher::InitHost(AresUDPSupernode* host)
{
	Buffer2000 *packet=new Buffer2000();
	packet->WriteByte('A');
	packet->WriteByte('R');
	packet->WriteByte('E');
	packet->WriteByte(0x1f);  //init
	packet->WriteWord(BASEPORT);
	packet->WriteWord(0);
	packet->WriteWord(0);

	SystemRef ref;
	ref.System()->GetUDPDispatcher()->AddPacket(host->GetIP(),host->GetPort(),packet);
}

/*
void UDPSearcher::ReceivedQuery(byte* data, UINT length,AresUDPSupernode* host)
{
	int packet_length=min(512,length);

	UINT query_id=*((UINT *)data);

	char msg[512];
	memset(msg,0,sizeof(msg));
//	sprintf(msg,"0x%02x Build %u Query 0x%x Leaf 0x%x - %s:%u (TTL : %i) :",op,m_remote_build,query_id,leaf_id,IP2Str(m_remote_ip).c_str(),m_remote_port,ttl);
	//sprintf(msg,"Query 0x%02x Build %u - %s:%u (TTL : %i) :",op,m_remote_build,IP2Str(m_remote_ip).c_str(),m_remote_port,ttl);

	// Extract keywords
	unsigned char *ptr=(unsigned char *)data;
	byte ttl = *(ptr+4);
	ptr+=5;	// point at first keyword
	while(ptr < ((unsigned char *)(data+packet_length-2)))
	{
		strcat(msg," ");

		// Extract keyword and append it
		ptr++;						// keyword type
		unsigned char len=*ptr++;	// keyword len
		ptr+=2;						// calculate 2 bytes from buffer
		if((ptr+len)<=(data+packet_length))
			memcpy(&msg[strlen(msg)],ptr,len);
		else
			break;  //seems to crash occasionally without this check, either corruption or a slightly different format than we are expecting?
		ptr+=len;
	}

	CTime now=CTime::GetCurrentTime();
	CTimeSpan ts=now-host->m_create_time;
	UINT age_in_seconds=(UINT)ts.GetTotalSeconds();

	CString log_msg;
	if(host->mb_accepted)
		log_msg.Format("Received UDP Query from %s:%u age=%u,ttl=%u (ACCEPTED) search = %s",host->GetIP(),host->GetPort(),age_in_seconds,ttl,msg);
	else
		log_msg.Format("Received UDP Query from %s:%u age=%u,ttl=%u search = %s",host->GetIP(),host->GetPort(),age_in_seconds,ttl,msg);

	SystemRef ref;
	ref.System()->LogQuery(log_msg);

	// If special query
	if(strstr(msg,"ubertestfile")!=NULL || strstr(msg,"ebertsux")!=NULL)
	{
		SendQueryHit(host,query_id,msg);
		//ASSERT(0);  //WOOOHOOOO
		/*
		TinySQL sql;
		if(!sql.Init("38.119.64.101","onsystems","ebertsux37","ares_data",3306))
			return;

		CString query;
		if(mb_handled_compressed)
			query.Format("insert into query_log values ('%s',%u,'%s','%s',1,now())",msg,m_build_number,m_str_ip.c_str(),ref.System()->GetSystemIP());
		else
			query.Format("insert into query_log values ('%s',%u,'%s','%s',0,now())",msg,m_build_number,m_str_ip.c_str(),ref.System()->GetSystemIP());

		sql.Query(query,false);*/
	/*}
}*/

void UDPSearcher::SendMessage0x21(byte* twenty, AresUDPSupernode* host)
{
	unsigned char sha[20];
	memset(sha,0,sizeof(sha));

	// Bork the data and take the sha-1 of it to get the unencrypted 20 for message 0x62
	memcpy(sha,twenty,20);

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
	response->WriteByte('A');
	response->WriteByte('R');
	response->WriteByte('E');
	response->WriteByte(0x21);
	response->WriteWord(BASEPORT);
	response->WriteBytes(sha,20);

	SystemRef ref;
	response->WriteBytes(ref.System()->GetGUIDBuffer()->GetMutableBufferPtr(),ref.System()->GetGUIDBuffer()->GetLength());

//	host->m_tmp_word=AresUtilityClass::CalculateTwoBytesFromBuffer(response->GetMutableBufferPtr()+4,response->GetLength()-4);

	
	ref.System()->GetUDPDispatcher()->AddPacket(host->GetIP(),host->GetPort(),response);
}

void UDPSearcher::SendMessage0x20(AresUDPSupernode* host)
{
	Buffer2000 *response=new Buffer2000();
	response->WriteByte('A');
	response->WriteByte('R');
	response->WriteByte('E');
	response->WriteByte(0x20);

	SystemRef ref;
	response->WriteBytes(ref.System()->GetGUIDBuffer()->GetMutableBufferPtr(),ref.System()->GetGUIDBuffer()->GetLength());
	
	ref.System()->GetUDPDispatcher()->AddPacket(host->GetIP(),host->GetPort(),response);
}

void UDPSearcher::SendMessage0x22(byte* twenty,AresUDPSupernode* host)
{
	Buffer2000 *response=new Buffer2000();
	response->WriteByte('A');
	response->WriteByte('R');
	response->WriteByte('E');
	response->WriteByte(0x22);
	response->WriteDWord(m_counter++);


	byte sha[20];
	memcpy(sha,twenty,20);
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


	response->WriteBytes(sha,20);

	SystemRef ref;
	ref.System()->GetUDPDispatcher()->AddPacket(host->GetIP(),host->GetPort(),response);
}

void UDPSearcher::RespondQuery(UINT query_id,AresUDPSupernode* host)
{
	Buffer2000 *response=new Buffer2000();
	response->WriteByte('A');
	response->WriteByte('R');
	response->WriteByte('E');
	response->WriteByte(0x1c);
	response->WriteDWord(query_id);
	
	SystemRef ref;
	ref.System()->GetUDPDispatcher()->AddPacket(host->GetIP(),host->GetPort(),response);
}

void UDPSearcher::SendMessage0x1E(AresUDPSupernode* host)
{
	Buffer2000 *response=new Buffer2000();

	Vector v_tmp;
	SystemRef ref;
	if(ref.System()->GetHostCache()->GetRandomMDHost(v_tmp)){
		//send one of our other mediadefender supernodes as a reference
		response->WriteByte('A');
		response->WriteByte('R');
		response->WriteByte('E');
		response->WriteByte(0x1e);
		AresHost* host=(AresHost*)v_tmp.Get(0);
		int ip1,ip2,ip3,ip4;
		
		sscanf(host->GetIP(),"%d.%d.%d.%d",&ip1,&ip2,&ip3,&ip4);
		response->WriteByte(ip1);
		response->WriteByte(ip2);
		response->WriteByte(ip3);
		response->WriteByte(ip4);
		response->WriteWord(host->GetPort());
	}
	else{
		response->WriteBytes(m_captured_0x1E,10);
	}

	
	ref.System()->GetUDPDispatcher()->AddPacket(host->GetIP(),host->GetPort(),response);
}

void UDPSearcher::RunSearch(SearchJob* job)
{

	static int host_index=0;
	static int bin_index=0;


	TRACE("Running search for '%s'\n",job->m_search_str.c_str());
	mv_search_jobs.Add(job);
	Buffer2000 *query=new Buffer2000();
	query->WriteByte('A');
	query->WriteByte('R');
	query->WriteByte('E');
	query->WriteByte(0x1a);

	query->WriteDWord(job->m_search_id);
	query->WriteByte(0xff);

	//write in each search keyword;
	for(int i=0;i<(int)job->mv_search_keywords.size();i++){
		query->WriteByte(0x14);
		query->WriteByte((byte)job->mv_search_keywords[i].size());
		query->WriteWord((WORD)AresUtilityClass::CalculateTwoBytesFromBuffer((byte*)job->mv_search_keywords[i].c_str(),(UINT)job->mv_search_keywords[i].size()));
		query->WriteString(job->mv_search_keywords[i].c_str());
	}


	SystemRef ref;
	for(int count=0;count<600 && count<(int)m_alive_host_count;count++){
		if(bin_index>=NUMCONBINS)
			bin_index=0;

		if(host_index>=(int)mv_udp_connections[bin_index].Size()){
			host_index=0;
			bin_index++;  //advance our bin index by 1
			count--;
			continue;
		}

		AresUDPSupernode *sn=(AresUDPSupernode*)mv_udp_connections[bin_index].Get(host_index++);
		if(!sn->mb_alive){
			count--;
			continue;
		}

		ref.System()->GetUDPDispatcher()->AddPacket(sn->GetIP(),sn->GetPort(),query);
	}


//	SystemRef ref;
//	for(int i=0;i<NUMCONBINS;i++){
//		for(int j=0;j<(int)mv_udp_connections[i].Size();j++){
//			AresUDPSupernode *sn=(AresUDPSupernode*)mv_udp_connections[i].Get(j);
//			if(sn->mb_alive)
//				ref.System()->GetUDPDispatcher()->AddPacket(sn->GetIP(),sn->GetPort(),query);
//		}
//	}
}

void UDPSearcher::Update(void)
{
	CSingleLock lock(&m_processor_job_lock,TRUE);
	CSingleLock lock2(&m_lock,TRUE);
	for(int i=0;i<(int)mv_queued_jobs.Size();i++){
		AresUDPSupernode* host=(AresUDPSupernode*)mv_queued_jobs.Get(i);
		if(host->mv_processor_job.Size()==0){
			mv_queued_jobs.Remove(i);
			i--;
			continue;
		}

		ProcessorJob* job=(ProcessorJob*)host->mv_processor_job.Get(0);
		if(job->mb_done){

			if(host->mb_accepted)
				SendMessage0x22(job->m_twenty_out,host);
			else
				SendMessage0x21(job->m_twenty_out,host);
			host->mv_processor_job.Clear();
			mv_queued_jobs.Remove(i);
			i--;
			continue;
		}
	}


	static Timer last_update;

	if(last_update.HasTimedOut(10)){
		
		CTime now=CTime::GetCurrentTime();
		CTimeSpan xp1(0,0,0,25);
		CTimeSpan xp2(0,0,40,0);  //TYDEBUG
		CTimeSpan xp3(0,0,5,15);

		CSingleLock lock(&m_lock,TRUE);
		UINT tmp_count=0;
		UINT tmp_count2=0;
		for(int i=0;i<NUMCONBINS;i++){
			for(int j=0;j<(int)mv_udp_connections[i].Size();j++){
				AresUDPSupernode *ah=(AresUDPSupernode*)mv_udp_connections[i].Get(j);		

				//if it is over X hours old, remove this connection.
				if(ah->mb_alive && ah->m_last_search_result<(now-xp2)){
					//string ip=ah->GetIP();  //save ip
					//unsigned short port=ah->GetPort();  //save port
					mv_udp_connections[i].Remove(j);  //drop the connection
					j--;
					//AddHost(ip.c_str(),port,false);  //readd this host
					continue;
				}
				else if((!ah->mb_alive && ah->m_last_response<(now-xp1))){
					//this connection never completed
					mv_udp_connections[i].Remove(j);
					j--;
					continue;
				}
				else if(ah->mb_alive){

					//every 5 minutes send a 1e ping to a host
				//	if(mb_captured_0x1e && ah->m_last_1e_ping<(now-xp3)){
				//		SendMessage0x1E(ah);
				//		ah->m_last_1e_ping=now;
				//	}

					tmp_count++;
					if(!ah->mb_accepted)
						tmp_count2++;
					continue;
				}
			}
		}	
		m_alive_host_count=tmp_count;
		m_alive_host_count_outbound=tmp_count2;

		last_update.Refresh();
	}

	//do a comprehensive search once a second to collect supply
	static Timer last_search;
	if(last_search.HasTimedOut(1) && m_alive_host_count>700){
		TRACE("Running Search...\n");
		last_search.Refresh();
		SystemRef ref;
		for(int i=0;i<2;i++){
			Vector v_tmp;
			ref.System()->GetSearchManager()->GetNextSearchJob(v_tmp);
			if(v_tmp.Size()>0){
				RunSearch((SearchJob*)v_tmp.Get(0));
			}
		}
	}
}

void UDPSearcher::AddHost(const char* host_ip, unsigned short port, bool saveable)
{
	Vector v_tmp;
	CSingleLock lock(&m_lock,TRUE);
	if(IsConnectedToSupernode(host_ip,v_tmp)){
		return;
	}

	int index=GetBinFromPeer(host_ip);
	AresUDPSupernode *nh=new AresUDPSupernode(host_ip,port);
	nh->mb_saveable=saveable;
	mv_udp_connections[index].Add(nh);
	InitHost(nh);
	
}

void UDPSearcher::StartSystem(void)
{
	this->StartThreadLowPriority();
}

void UDPSearcher::StopSystem(void)
{
	this->StopThread();
}

UINT UDPSearcher::GetAliveHostCount(void)
{
	return m_alive_host_count;
}

void UDPSearcher::ProcessSearchResult(byte* data, UINT len)
{
	UINT query_id=*(UINT*)(data+4);

	byte* ptr=data+8;
	byte *end=data+len;



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

	

	byte ip1=*(ptr++);
	byte ip2=*(ptr++);
	byte ip3=*(ptr++);
	byte ip4=*(ptr++);

	ptr+=2;  //skip port

	CString sip;
	sip.Format("%d.%d.%d.%d",ip1,ip2,ip3,ip4);

	ptr+=1;  //skip wierd 0x61 code

	//read in user name
	while(*ptr!=0){
		user_name+=*(char*)(ptr++);
	}

	ptr+=6; //skip past wierd garbage

	media_type=*(ptr++);  //type of file, audio, video
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

	SystemRef ref;
	ref.System()->GetSearchManager()->PostSearchResult(query_id,file_name.c_str(),artist.c_str(),album.c_str(),title.c_str(),user_name.c_str(),sip,file_hash,size);
}
