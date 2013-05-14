#include "StdAfx.h"
#include "supernodeudpsystem.h"
#include "AresSupernodeSystem.h"
#include "..\tkcom\Buffer2000.h"
#include "..\AresDataCollector\ProcessorJob.h"
#include "..\tkcom\tinysql.h"
#include "AresUserName.h"
#include "..\AresProtector\AresUtilityClass.h"

SupernodeUDPSystem::SupernodeUDPSystem(void)
{
	m_alive_host_count=0;
	m_alive_host_count_outbound=0;
	AfxSocketInit();
	int i=0;
	//for(int i=0;i<NUM_PORTS;i++){
		BOOL stat=m_rcv_socket.Create(BASEPORT,SOCK_DGRAM,0,NULL);
		if(!stat){
			int error=GetLastError();
			TRACE("Couldn't create UDP Supernode Socket %d, error = %d\n",i,error);
		}
	//}

	mb_captured_0x1e=false;

	//generate 50 common spoof hashes
	srand(10);

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
	}

	m_forward_index1=0;
	m_forward_index2=0;

	m_counter=50+rand()%5000;
}

SupernodeUDPSystem::~SupernodeUDPSystem(void)
{
}

UINT SupernodeUDPSystem::Run(void)
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


		if(op_code!=0x22 && op_code!=0x1f && op_code!=0x20 && op_code!=0x1a && op_code!=0x1e && op_code!=0x1c && op_code!=0x1b && op_code!=0x21){
			TRACE("UDP RECEIVE OPCODE 0x%X length %u from %s:%u\n",op_code,nread,addr,port);
		}

		AresSNUDPHost *host=NULL;
		if(v_tmp.Size()>0){
			host=(AresSNUDPHost*)v_tmp.Get(0);
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

				
				AresSupernodeSystemRef ref;
				int index=ref.System()->GetBinFromPeer(addr);
				AresSNUDPHost *nh=new AresSNUDPHost(addr,tmp_port1);
				mv_udp_connections[index].Add(nh);
				nh->mb_accepted=true;
				SendMessage0x20(nh);  

				ref.System()->SaveHost(addr,tmp_port1);
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
			AresSupernodeSystemRef ref;
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

			AresSupernodeSystemRef ref;
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
			AresSupernodeSystemRef ref;
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
			if(new_port!=4685 && new_port!=BASEPORT){
				//AddHost(new_ip,new_port,true);
				AresSupernodeSystemRef ref;
				ref.System()->SaveHost(new_ip,new_port);
			}

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
			UINT fake_query_id=*(UINT*)(buf+4);

			
			CTime now=CTime::GetCurrentTime();
			CTimeSpan xp1(0,0,1,0);
			for(int i=0;i<(int)mv_waiting_queries.Size();i++){
				WaitingQuery *wq=(WaitingQuery*)mv_waiting_queries.Get(i);
				if(wq->m_fake_query_id==fake_query_id){
					//send this on to the original searcher
					ForwardQueryHit(wq->mp_host,wq->m_real_query_id,buf,nread);
					continue;
				}
				else if(wq->m_create_time<(now-xp1)){  //this waiting query is old, lets purge it
					mv_waiting_queries.Remove(i);
					i--;
					continue;
				}
			}
			
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
			
			if(!host->mb_alive)
				m_alive_host_count++;

			TRACE("UDP QUERY:  %s:\n",addr);
			for(int i=0;i<nread;i++){
				byte b=buf[i];
				if(b!=0){
					TRACE("%d 0x%.2X '%c' \n",i,b,b);
				}
				else{
					TRACE("%d 0x%.2X NULL \n",i,b,b);
				}
			}

			host->mb_alive=true;
			host->m_last_response=CTime::GetCurrentTime();
			host->m_last_search=CTime::GetCurrentTime();

			//TEST ********  FORWARD QUERY TO RANDOM HOST AND LOOK AT RESPONSE

			UINT real_query_id=*(UINT*)(buf+4);

			RespondQuery(real_query_id,host); //immediately send back the 0x1c message

			//lets not worry about this part, we don't care about udp queries
			//ReceivedQuery(buf+4,nread-4,host);

			buf[4]=rand()%4;
			buf[5]=rand()%4;
			buf[6]=rand()%4;
			buf[7]=rand()%4;

			UINT fake_query_id=*(UINT*)(buf+4);

			*(byte*)(buf+8)=255;
			
			//we want to answer this original query when/if we get results for it
			//mv_waiting_queries.Add(new WaitingQuery(host,fake_query_id,real_query_id));

			CSingleLock lock(&m_lock,TRUE);

			//TRACE("Forwarding query_id fake %u.\n",fake_query_id);
			//i need to be an active searcher, this allows me to pretend that i am a real supernode as well by returning search results
			
			/*
			for(int forward_index=0;forward_index<15 && forward_index<(int)m_alive_host_count;forward_index++){
				if(m_forward_index1>=NUMCONBINS)
					m_forward_index1=0;

				if(m_forward_index2>=mv_udp_connections[m_forward_index1].Size()){
					m_forward_index2=0;
					m_forward_index1++;  //advance our bin index by 1
				}
				else{
					AresSNUDPHost* tmp_host=(AresSNUDPHost*)mv_udp_connections[m_forward_index1].Get(m_forward_index2++);
					if(host->mb_alive){
						//TRACE("Query forwarded to %s:%u\n",host->GetIP(),host->GetPort());
						ForwardQuery(tmp_host,buf,nread);
						if(mb_captured_0x1e && tmp_host->m_last_1e_ping<(CTime::GetCurrentTime()-CTimeSpan(0,0,5,0))){
							tmp_host->m_last_1e_ping=CTime::GetCurrentTime();
							SendMessage0x1E(tmp_host);
						}
					}
				}
			}*/
		}
	}
	return 0;
}

void SupernodeUDPSystem::Update(void)
{
	CSingleLock lock(&m_processor_job_lock,TRUE);
	for(int i=0;i<(int)mv_queued_jobs.Size();i++){
		AresSNUDPHost* host=(AresSNUDPHost*)mv_queued_jobs.Get(i);
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
				AresSNUDPHost *ah=(AresSNUDPHost*)mv_udp_connections[i].Get(j);		

				//if it is over a X hours old, remove this connection.
				if(ah->mb_alive && ah->m_last_search<(now-xp2)){
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
}

void SupernodeUDPSystem::RespondQuery(UINT query_id,AresSNUDPHost* host)
{
	Buffer2000 *response=new Buffer2000();
	response->WriteByte('A');
	response->WriteByte('R');
	response->WriteByte('E');
	response->WriteByte(0x1c);
	response->WriteDWord(query_id);
	
	AresSupernodeSystemRef ref;
	ref.System()->GetUDPDispatcher()->AddPacket(host,response);
}

void SupernodeUDPSystem::StartSystem(void)
{
	this->StartThreadLowPriority();
}

void SupernodeUDPSystem::StopSystem(void)
{
	m_rcv_socket.Close();
	this->StopThread();
}

bool SupernodeUDPSystem::IsConnectedToSupernode(const char* host_ip, Vector& v_tmp)
{
	AresSupernodeSystemRef ref;
	int index=ref.System()->GetBinFromPeer(host_ip);
	CSingleLock lock(&m_lock,TRUE);
	for(int i=0;i<(int)mv_udp_connections[index].Size();i++){
		AresSNUDPHost *ah=(AresSNUDPHost*)mv_udp_connections[index].Get(i);

		if(stricmp(ah->GetIP(),host_ip)==0){
			v_tmp.Add(ah);
			return true;
		}
	}

	return false;
}

void SupernodeUDPSystem::AddHost(const char* host_ip, unsigned short port, bool saveable)
{
	Vector v_tmp;
	CSingleLock lock(&m_lock,TRUE);
	if(IsConnectedToSupernode(host_ip,v_tmp)){
		return;
	}

	AresSupernodeSystemRef ref;
	int index=ref.System()->GetBinFromPeer(host_ip);
	AresSNUDPHost *nh=new AresSNUDPHost(host_ip,port);
	nh->mb_saveable=saveable;
	mv_udp_connections[index].Add(nh);
	InitHost(nh);
	
}

void SupernodeUDPSystem::InitHost(AresSNUDPHost* host)
{
	Buffer2000 *packet=new Buffer2000();
	packet->WriteByte('A');
	packet->WriteByte('R');
	packet->WriteByte('E');
	packet->WriteByte(0x1f);  //init
	packet->WriteWord(BASEPORT);
	packet->WriteWord(0);
	packet->WriteWord(0);

	AresSupernodeSystemRef ref;
	ref.System()->GetUDPDispatcher()->AddPacket(host,packet);
}

void SupernodeUDPSystem::ReceivedQuery(byte* data, UINT length,AresSNUDPHost* host)
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

	AresSupernodeSystemRef ref;
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
	}
}

void SupernodeUDPSystem::SendMessage0x21(byte* twenty, AresSNUDPHost* host)
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

	AresSupernodeSystemRef ref;
	response->WriteBytes(ref.System()->GetGUIDBuffer()->GetMutableBufferPtr(),ref.System()->GetGUIDBuffer()->GetLength());

//	host->m_tmp_word=AresUtilityClass::CalculateTwoBytesFromBuffer(response->GetMutableBufferPtr()+4,response->GetLength()-4);

	
	ref.System()->GetUDPDispatcher()->AddPacket(host,response);
}

UINT SupernodeUDPSystem::GetAliveHostCount(void)
{
	return m_alive_host_count;
}


void SupernodeUDPSystem::SendMessage0x20(AresSNUDPHost* host)
{
	Buffer2000 *response=new Buffer2000();
	response->WriteByte('A');
	response->WriteByte('R');
	response->WriteByte('E');
	response->WriteByte(0x20);

	AresSupernodeSystemRef ref;
	response->WriteBytes(ref.System()->GetGUIDBuffer()->GetMutableBufferPtr(),ref.System()->GetGUIDBuffer()->GetLength());
	
	ref.System()->GetUDPDispatcher()->AddPacket(host,response);
}

void SupernodeUDPSystem::SendMessage0x22(byte* twenty,AresSNUDPHost* host)
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

	AresSupernodeSystemRef ref;
	ref.System()->GetUDPDispatcher()->AddPacket(host,response);
}

void SupernodeUDPSystem::DoQuery(void)
{
}

void SupernodeUDPSystem::ForwardQuery(AresSNUDPHost* host,byte* data, UINT length)
{
	//randomize our search id
	AresSupernodeSystemRef ref;
	ref.System()->GetUDPDispatcher()->AddPacket(host,new Buffer2000(data,length));
}

void SupernodeUDPSystem::SendQueryHit(AresSNUDPHost* host,UINT query_id, const char* query_str)
{
	//generate 100 hashes that we are going to send based on query_str;
	int sum_str=0;
	int str_len=(int)strlen(query_str);
	for(int i=0;i<str_len;i++){
		sum_str+=(int)query_str[i];
	}

	srand(sum_str);

	AresSupernodeSystemRef ref;
	CString file_name;
	file_name.Format("ubertestfile ebertsuxudp sn %s srv %s.mp3",host->GetIP(),ref.System()->GetSystemIP());

	//load up X user names to spoof
	Vector v_users;
	ref.System()->GetHostCache()->GetSomeUserNames(m_total_results,v_users);

	int name_index=0;

	Vector v_packets;

	if(rand()&1){
		for(int hash_index=0;hash_index<NUM_SPOOF_HASHES && v_users.Size()>0;hash_index++){
			for(int result_index=0;result_index<m_num_results[hash_index];result_index++){
				AresUserName *user_name=(AresUserName*)v_users.Get(((name_index++)%v_users.Size()));

				Buffer2000 *packet=new Buffer2000();
				packet->WriteByte('A');
				packet->WriteByte('R');
				packet->WriteByte('E');
				packet->WriteByte(0x1b);  //query hit
				packet->WriteDWord(query_id);
				packet->WriteWord(rand()); //write ip
				packet->WriteWord(rand()); //write ip
				packet->WriteWord(5000+(rand()%10000)); //write port
				packet->WriteByte(0x61);  //wierd code
				packet->WriteString(user_name->m_name.c_str());  //USER NAME
				packet->WriteByte(0);  //NULL
				packet->WriteByte(0x10);  //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0);     //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0);     //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0x04);  //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0);  //NULL
				packet->WriteByte(0x01);  //type audio
				packet->WriteDWord(7654321);  //size

				//write in sha1
				packet->WriteBytes(m_spoof_hashes[hash_index],20);

				packet->WriteByte('.');  //extension
				packet->WriteByte('m');  //extension
				packet->WriteByte('p');  //extension
				packet->WriteByte('3');  //extension
				packet->WriteByte(0);    //NULL

				//title
				packet->WriteByte(0x01);
				packet->WriteString("Ubertestfile ebertsux");
				packet->WriteByte(0);

				//artist
				packet->WriteByte(0x02);
				packet->WriteString("Ubertestfile ebertsux udpartist");
				packet->WriteByte(0);

				//bitrate,medialength
				packet->WriteByte(0x04);
				packet->WriteWord(192);
				packet->WriteDWord(777);

				//filename
				packet->WriteByte(0x10);
				packet->WriteString((LPCSTR)file_name);
				packet->WriteByte(0x00);

				//queue this packet to be sent
				v_packets.Add(packet);
				//ref.System()->GetUDPDispatcher()->AddPacket(host,packet);
			}
		}
	}
	else{
		for(int hash_index=NUM_SPOOF_HASHES-1;hash_index>=0 && v_users.Size()>0;hash_index--){
			for(int result_index=0;result_index<m_num_results[hash_index];result_index++){
				AresUserName *user_name=(AresUserName*)v_users.Get(((name_index++)%v_users.Size()));

				Buffer2000 *packet=new Buffer2000();
				packet->WriteByte('A');
				packet->WriteByte('R');
				packet->WriteByte('E');
				packet->WriteByte(0x1b);  //query hit
				packet->WriteDWord(query_id);
				packet->WriteWord(rand()); //write ip
				packet->WriteWord(rand()); //write ip
				packet->WriteWord(5000+(rand()%10000)); //write port
				packet->WriteByte(0x61);  //wierd code
				packet->WriteString(user_name->m_name.c_str());  //USER NAME
				packet->WriteByte(0);  //NULL
				packet->WriteByte(0x10);  //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0);     //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0);     //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0x04);  //FIGURE OUT WHAT THIS IS
				packet->WriteByte(0);  //NULL
				packet->WriteByte(0x01);  //type audio
				packet->WriteDWord(7654321);  //size

				//write in sha1
				packet->WriteBytes(m_spoof_hashes[hash_index],20);

				packet->WriteByte('.');  //extension
				packet->WriteByte('m');  //extension
				packet->WriteByte('p');  //extension
				packet->WriteByte('3');  //extension
				packet->WriteByte(0);    //NULL

				//title
				packet->WriteByte(0x01);
				packet->WriteString("Ubertestfile ebertsux");
				packet->WriteByte(0);

				//artist
				packet->WriteByte(0x02);
				packet->WriteString("Ubertestfile ebertsux udpartist");
				packet->WriteByte(0);

				//bitrate,medialength
				packet->WriteByte(0x04);
				packet->WriteWord(192);
				packet->WriteDWord(777);

				//filename
				packet->WriteByte(0x10);
				packet->WriteString((LPCSTR)file_name);
				packet->WriteByte(0x00);

				//queue this packet to be sent
				v_packets.Add(packet);
				//ref.System()->GetUDPDispatcher()->AddPacket(host,packet);
			}
		}
	}

	//send them randomly so we don't bunch the results and make it look fishy
	while(v_packets.Size()>0){
		int index=rand()%v_packets.Size();
		ref.System()->GetUDPDispatcher()->AddPacket(host,(Buffer2000*)v_packets.Get(index));
		v_packets.Remove(index);
	}


	//TYDEBUG
	/* 
	TinySQL sql;
	if(!sql.Init("38.119.64.101","onsystems","ebertsux37","ares_data",3306))
		return;

	CString query;
	query.Format("insert into query_log values ('%s',%u,'%s','%s',0,now())",query_str,1,host->GetIP(),ref.System()->GetSystemIP());

	sql.Query(query,false);
	*/
}

void SupernodeUDPSystem::SendMessage0x1E(AresSNUDPHost* host)
{
	Buffer2000 *response=new Buffer2000();

	Vector v_tmp;
	AresSupernodeSystemRef ref;
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

	
	ref.System()->GetUDPDispatcher()->AddPacket(host,response);
}

void SupernodeUDPSystem::ForwardQueryHit(AresSNUDPHost* host, UINT real_query_id, byte* data, UINT len)
{
	//change the query_id back to the original one, and forward it to the original querier
	*(UINT*)(data+4)=real_query_id;

	Buffer2000 *response=new Buffer2000();
	response->WriteBytes(data,len);

	AresSupernodeSystemRef ref;
	ref.System()->GetUDPDispatcher()->AddPacket(host,response);
}

UINT SupernodeUDPSystem::GetAliveHostCountOutbound(void)
{
	return m_alive_host_count_outbound;
}
