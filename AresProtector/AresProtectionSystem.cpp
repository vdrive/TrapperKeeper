#include "StdAfx.h"
#include "aresprotectionsystem.h"
#include "..\tkcom\Timer.h"
#include ".\aresprotectionsystem.h"
#include "..\tkcom\FrameHeader.h"
#include <mmsystem.h>
#include "..\tkcom\sha1.h"
#include "DataSendEvent.h"
#include "AresFTBanClient.h"


#define NOISE_BUFFER_LEN	((4*1024)+1)	// 4K + 1
#define NOISE_DATA_LEN		(1<<21)-NOISE_BUFFER_LEN	// 2 MB

//static variable initialization
UINT AresProtectionSystem::sm_system_count=0;
AresProtectionSystem* AresProtectionSystem::sm_system=NULL;
CCriticalSection AresProtectionSystem::sm_system_lock;

AresProtectionSystem::AresProtectionSystem()
{
	mb_processor_ready=false;
	m_event_cache_time=1;
	mb_force_connection_purge=false;
	m_status="Operational - Generating Files";
	mb_has_project_info=false;
	m_connecting_sockets=0;

	mp_processor=NULL;

	m_interdiction_transfer_amount=0;

	m_num_host_connections=0;
	m_num_alive_host_connections=0;
	m_num_ft_connections=0;
	m_num_interdiction_connections=0;
	m_num_interdiction_queued=0;

	m_total_hash_cache_hits=0;
	m_found_hash_cache_hits=0;
}

AresProtectionSystem::~AresProtectionSystem(void)
{
	if(mp_processor!=NULL)
		delete mp_processor;
}

UINT AresProtectionSystem::Update(){

	//m_interdiction_manager.Update();  //TYDEBUG

	if(!mb_has_project_info){
		if(m_ares_project_info_cache.IsReady()){
			mb_has_project_info=true;
			m_ares_spoof_cache.StartThread();  //one time code, we can now begin loading spoofs if we have project info
		}
	}

	if(!mb_processor_ready || !m_ares_spoof_cache.IsReady() || !m_ares_project_info_cache.IsReady())
		return 0;
	
	if(m_num_host_connections==0)  //only first time through do we set this
		m_status="Operational - Uploading Files To Hosts";

	bool b_can_clean=false;

	if(m_num_host_connections>1350 || m_num_ft_connections>600 || m_connection_purge_timer.HasTimedOut(15) || m_connection_purge_timer.HasTimedOut(60*30) || mb_force_connection_purge){
		b_can_clean=true;
		mb_force_connection_purge=false;
		m_connection_purge_timer.Refresh();
	}

	static Timer last_dying_update;

	//see if there are any clients ready to be removed from our master linked list
	if(last_dying_update.HasTimedOut(5) || b_can_clean){
		last_dying_update.Refresh();

		for(int i=0;i<NUMCONBINS;i++){
			for(int j=0;j<(int)mv_host_connections[i].Size();j++){
				AresConnection *ac=(AresConnection*)mv_host_connections[i].Get(j);
				
				if(ac->IsDying()){
					ac->Update();  //see if it is dead yet
				}

				bool b_dead=ac->IsDead();

				if(b_can_clean && b_dead){
					//TRACE("AresProtectionSystem::Update() removing host connection %s:%u\n",ac->GetIP(),ac->GetPort());
					ac->CleanUp();
					mv_host_connections[i].Remove(j);
					m_num_host_connections--;
					j--;
					continue;
				}
			}
		}

		/*
		ml_host_connections.StartIteration();
		while(true){
			AresConnection *ac=(AresConnection*)ml_host_connections.GetCurrent();
			if(!ac)
				break;
			
			if(ac->IsDying()){
				ac->Update();  //see if it is dead yet
			}

			bool b_dead=ac->IsDead();

			if(b_can_clean && b_dead){
				//TRACE("AresProtectionSystem::Update() removing host connection %s:%u\n",ac->GetIP(),ac->GetPort());
				ac->CleanUp();
				ml_host_connections.RemoveCurrentAndAdvance();
				continue;
			}

			ml_host_connections.Advance();
		}*/
	}

	static Timer last_living_update;

	//give special considering to client connection objects that are not disconnected
	if(last_living_update.HasTimedOut(3) || b_can_clean){
		last_living_update.Refresh();

		for(int i=0;i<NUMCONBINS;i++){
			for(int j=0;j<(int)mv_alive_host_connections[i].Size();j++){
				AresConnection *ac=(AresConnection*)mv_alive_host_connections[i].Get(j);
				
				if(ac->IsDying()){
					mv_alive_host_connections[i].Remove(j);
					m_num_alive_host_connections--;
					j--;
					continue;
				}
				else{
					ac->Update();
				}
			}
		}
	}

	//initiated new connections
	//  TYDEBUG
	
	for(int i=0;i<7 && m_connecting_sockets<10 && mv_processor_jobs.Size()<2 && m_num_alive_host_connections<550;i++){  //open X new connections per update
		AresHost *host=m_host_cache.GetNextHost();
		if(host){
			if(!IsConnectedToUltraPeer(host->GetIP())){
				//giFT = port 2597
				//gnutella = port 6346
				//we are not already connected to this ultra peer so lets open a connection
				int index=GetBinFromPeer(host->GetIP());
				AresConnection *new_con=new AresConnection(host,30500+rand()%30);  //TYDEBUG
				mv_host_connections[index].Add(new_con);
				mv_alive_host_connections[index].Add(new_con);
				m_num_alive_host_connections++;
				m_num_host_connections++;
				new_con->Connect();
			}
		}
	}

	return 1;
}

void AresProtectionSystem::StartSystem(void)
{
//	InitUserName();
	m_decoy_pool.StartThreadLowPriority();
	InitMP3Noise();
	m_host_cache.StartThread();
	
	UINT ipCount=0;
	//char *ip;
	PHOSTENT hostinfo;
	char hostname[1024];

	//in_addr hostIPs[128];
	if( gethostname ( hostname, sizeof(hostname)) == 0)
	{
		TRACE("Host name: %s\n", hostname);

		if((hostinfo = gethostbyname(hostname)) != NULL)
		{
			//int nCount = 0;
			while(hostinfo->h_addr_list[ipCount] && ipCount<MAXSYSTEMIPS)
			{
				mv_system_ips.push_back(inet_ntoa(*(struct in_addr *)hostinfo->h_addr_list[ipCount]));
				//memcpy(&hostIPs[ipCount],(struct in_addr *)hostinfo->h_addr_list[ipCount],sizeof(in_addr));
				
				TRACE("SpoofingSystem IP #%d: %s\n", ipCount, mv_system_ips[ipCount].c_str());
				++ipCount;
			}
		}
	}


	for(int i=0;i<30;i++){
		mv_servers.push_back(m_tcp_system.Listen(30500+i));
		//ServerSocket *s=new ServerSocket();
		//s->Init(30500+i,this);
		//ml_servers.Add(s);
	}
	

	m_ares_project_info_cache.StartThread();

	CleanDirectory("c:\\Ares_Cache");
	CreateDirectory("c:\\Ares_Cache",NULL);
	

	this->StartThreadLowPriority();

//	m_interdiction_manager.StartSystem();  //TYDEBUG


	//WSADATA wsdata;
	//memset(&wsdata,0,sizeof(WSADATA));
	//WSAStartup(0x0202,&wsdata);
	
	//this->StartThread();
}

void AresProtectionSystem::StopSystem(void)
{
	m_decoy_pool.b_killThread=1;
	for(int i=0;i<NUMCONBINS;i++){
		for(int j=0;j<(int)mv_host_connections[i].Size();j++){
			AresConnection *ac=(AresConnection*)mv_host_connections[i].Get(j);
			
			ac->CleanUp();
		}
		mv_host_connections[i].Clear();
	}

	for(int i=0;i<NUMCONBINS;i++){
		for(int j=0;j<(int)mv_alive_host_connections[i].Size();j++){
			AresConnection *ac=(AresConnection*)mv_alive_host_connections[i].Get(j);
			
			ac->CleanUp();
		}
		mv_alive_host_connections[i].Clear();
	}

	for(int i=0;i<NUMCONBINS;i++){
		for(int j=0;j<(int)mv_ft_connections[i].Size();j++){
			AresFTConnection *ac=(AresFTConnection*)mv_ft_connections[i].Get(j);
			
			ac->CleanUp();
		}
		mv_ft_connections[i].Clear();
	}

	/*
	ml_servers.StartIteration();
	while(true){
		ServerSocket *ac=(ServerSocket*)ml_servers.GetCurrent();
		if(!ac)
			break;

		ac->Close();
		
		ml_servers.RemoveCurrentAndAdvance();
	}*/

	m_host_cache.StopThread();
	m_ares_spoof_cache.StopThread();

	this->StopThread();

	m_tcp_system.StopSystem();

	m_interdiction_manager.StopSystem();
	
	m_decoy_pool.StopThread();
	return;
	
	//this->StopThread();
}

void AresProtectionSystem::SetEventCacheTime(int val)
{
	m_event_cache_time=val;
}

int AresProtectionSystem::GetEventCacheTime(void)
{
	return m_event_cache_time;
}

Vector* AresProtectionSystem::GetConnectedHosts(void)
{
	return mv_host_connections;
}

AresDecoyPool* AresProtectionSystem::GetDecoyPool(){
	return &m_decoy_pool;
}

AresConnection* AresProtectionSystem::GetHost(UINT id)
{
	for(int i=0;i<NUMCONBINS;i++){
		for(int j=0;j<(int)mv_host_connections[i].Size();j++){
			AresConnection *ac=(AresConnection*)mv_host_connections[i].Get(j);
			if(ac->GetID()==id)
				return ac;			
		}
	}

	return NULL;
}

AresHostCache* AresProtectionSystem::GetHostCache(void)
{
	return &m_host_cache;
}

void AresProtectionSystem::ResetConnectionPurgeTimer(void)
{
	m_connection_purge_timer.Refresh();
}

bool AresProtectionSystem::IsConnectedToUltraPeer(const char* host_ip)
{
	int index=GetBinFromPeer(host_ip);

	for(int i=0;i<(int)mv_alive_host_connections[index].Size();i++){
		AresConnection *ac=(AresConnection*)mv_alive_host_connections[index].Get(i);

		if(stricmp(ac->GetIP(),host_ip)==0)
			return true;
	}

	return false;
}

void AresProtectionSystem::ForceConnectionPurge(void)
{
	mb_force_connection_purge=true;
}

void AresProtectionSystem::GetSomeSpoofs(Vector& v_file_group)
{
	//Vector v_spoofs;
	//m_ares_spoof_cache.GetSpoofs(v_spoofs);
	CSingleLock lock(&m_ares_spoof_cache.m_lock,TRUE);

	if(m_ares_spoof_cache.mv_file_groups.Size()<1)
		return;

	int rand_offset=rand()%m_ares_spoof_cache.mv_file_groups.Size();

	v_file_group.Add(m_ares_spoof_cache.mv_file_groups.Get(rand_offset));
	//v_files.Copy((Vector*)m_ares_spoof_cache.mv_file_groups.Get(rand_offset));
	//int count=0;
	//for(int i=rand_offset ; count<(int)m_ares_spoof_cache.mv_ares_files.Size() && count<limit ; i++,count++){
	//	if((rand()%100) < chance){	//a random chance this decoy will be included
	//		v_files.Add(m_ares_spoof_cache.mv_ares_files.Get(i%m_ares_spoof_cache.mv_ares_files.Size()));  
	//	}
	//}
}


AresProjectInfoCache* AresProtectionSystem::GetProjectInfoCache(void)
{
	return &m_ares_project_info_cache;
}

void AresProtectionSystem::UpdateConnecting(int dif)
{
	m_connecting_sockets+=dif;
//	ASSERT(m_connecting_sockets>=0 && m_connecting_sockets<=100);
}

unsigned int AresProtectionSystem::GetSpoofsLoaded(void)
{
	return m_ares_spoof_cache.GetSpoofCount();;
}

void AresProtectionSystem::OnAccept(UINT server_handle,UINT con_id,const char* peer,unsigned short port){

	//TYDEBUG
	TRACE("AresProtectionSystem::OnAccept() Start %s:%u\n",peer,port);
//	if(ml_ft_connections.Size()>400){
		//get rid of oldest connection
//		ml_ft_connections.StartIteration();
//		AresFTConnection *ac=(AresFTConnection*)ml_ft_connections.GetCurrent();
//		if(ac){
//			ac->CleanUp();
//			ml_ft_connections.RemoveCurrentAndAdvance();
//		}
//	}
	
	bool b_ip_filter=true;  //TOGGLE TO TURN ON/OFF IP FILTERING

	if(b_ip_filter){
		//PeerSocket *ps=(PeerSocket*)new_connection;
		if(!IsIPOkay(peer)){  //TYDEBUG
			//only serve data to a select set of IPs
			m_tcp_system.CloseConnection(con_id);  //we are not interested in this connection at this time
			TRACE("AresProtectionSystem::OnAccept() Finish 1 %s:%u\n",peer,port);
			return;
		}
	}

	AresFTConnection *nc=new AresFTConnection(con_id,peer,port);
	int index=GetBinFromPeer(peer);
	mv_ft_connections[index].Add(nc);
	TRACE("AresProtectionSystem::OnAccept() Finish 2 %s:%u\n",peer,port);
}

Vector* AresProtectionSystem::GetFTConnections(void)
{
	return mv_ft_connections;
}

UINT AresProtectionSystem::GetFileLength(byte* hash)
{
	return m_ares_spoof_cache.GetFileLength(hash);
}

void AresProtectionSystem::InitMP3Noise(void)
{
	srand(timeGetTime());

	srand(50+rand()%2); //seed to one in X possible outcomes

	// Else use random noise
	int i;

	byte* p_noise_data_buf=new byte[NOISE_DATA_LEN+NOISE_BUFFER_LEN];
	memset(p_noise_data_buf,0,NOISE_DATA_LEN+NOISE_BUFFER_LEN);
	
	FrameHeader hdr(32,44.1);
	unsigned int frame_len=hdr.ReturnFrameSize();

	unsigned int offset=0;
	while(1)
	{
		if((NOISE_DATA_LEN-offset)<sizeof(FrameHeader))
		{
			break;
		}
		memcpy(&p_noise_data_buf[offset],&hdr,sizeof(FrameHeader));
		offset+=sizeof(FrameHeader);

		if((NOISE_DATA_LEN-offset)<frame_len)
		{
			break;
		}
			
		for(i=0;i<(int)frame_len-(int)sizeof(FrameHeader);i++)
		{
//			p_noise_data_buf[offset+i]=(i+rand()%2)%256;	// noisy hell
//			p_noise_data_buf[offset+i]=rand()%256;			// less frequent noise (on winamp, but not on mediaplayer)
			
			p_noise_data_buf[offset+i]=rand()%12;			// quiet noise
		}

		offset+=frame_len-sizeof(FrameHeader);
	}

	// Init remainder of dookie
	for(i=offset;i<NOISE_DATA_LEN;i++)
	{
		p_noise_data_buf[i]=rand()%256;
	}

	// Copy out the buffer padding
	memcpy(&p_noise_data_buf[NOISE_DATA_LEN],&p_noise_data_buf[0],NOISE_BUFFER_LEN);

	m_mp3_noise.WriteBytes(p_noise_data_buf,NOISE_DATA_LEN+NOISE_BUFFER_LEN);

	delete []p_noise_data_buf;


	//setup data shas
/*
	SHA1 sha_256;
	//SHA1 sha_1_meg;
	//SHA1 sha_2_meg;
	//SHA1 sha_5_meg;

	//do 256
	sha_256.Update(m_mp3_noise.GetMutableBufferPtr(),m_mp3_noise.GetLength());

	//do 1 meg
	//for(int i=0;i<4;i++)
	//	sha_1_meg.Update(m_mp3_noise.GetMutableBufferPtr(),m_mp3_noise.GetLength());

	//do 2 meg
	//for(int i=0;i<4*2;i++)
	//	sha_2_meg.Update(m_mp3_noise.GetMutableBufferPtr(),m_mp3_noise.GetLength());

	//do 5 meg
	//for(int i=0;i<4*5;i++)
	//	sha_5_meg.Update(m_mp3_noise.GetMutableBufferPtr(),m_mp3_noise.GetLength());

	sha_256.Final();
	//sha_1_meg.Final();
	//sha_2_meg.Final();
	//sha_5_meg.Final();

	sha_256.GetHash(m_data_256K_hash);
	//sha_1_meg.GetHash(m_data_1Meg_hash);
	//sha_2_meg.GetHash(m_data_2Meg_hash);
	//sha_5_meg.GetHash(m_data_5Meg_hash);
	*/

	srand(timeGetTime());
	
}

Buffer2000* AresProtectionSystem::GetMP3NoiseBuffer(void)
{
	return &m_mp3_noise;
}

const char* AresProtectionSystem::GetSystemIP(void)
{
	if(mv_system_ips.size()>0){
		return mv_system_ips[0].c_str();
	}
	else return "5.6.7.8";
}

UINT AresProtectionSystem::Run(){

	for(int i=0;i<LOADDELAY && !this->b_killThread;i++){ //sleep for X seconds before we start the processor (in case the assembly.bin file hasn't quite finished synching yet or its causing it to crash for some reason, at least we'll be able to patch before TK crashes)
		Sleep(1000);  
	}
	if(!this->b_killThread){
		TRACE("Loading processor BEGIN...\n");
		mp_processor=new Processor();
		TRACE("Loading processor DONE\n");
		mb_processor_ready=true;
	}
	while(!this->b_killThread){
		Sleep(300);
		if(mv_processor_jobs.Size()>0){
//			Log("Ares Processor:  Started processing 0x38 message....");
			ProcessorJob* job=(ProcessorJob*)mv_processor_jobs.Get(0);
			DWORD time1=GetTickCount();
			mp_processor->ProcessData(job->m_sixteen_in,job->m_twenty_out);

			DWORD time2=GetTickCount();
			job->mb_done=true;  //signal this job as having been completed

			mv_processor_jobs.Remove(0);

			double dtime1=(double)time1;
			double dtime2=(double)time2;

			double seconds=(dtime2-dtime1)/1000.0;

//			CString log_msg;
//			log_msg.Format("Ares Processor:  Finished processing 0x38 message in %.2f seconds!",(float)seconds);
//			Log(log_msg);
		}
	}
	return 0;
}

void AresProtectionSystem::CleanDirectory(const char* directory)
{
	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	
	//make damn sure we are working in the syncher's or the fasttrack directory.
	if(path.Find("c:\\syncher")==-1 && path.Find("c:\\metamachine")==-1)  
		return;

	WIN32_FIND_DATA info;
	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		bool b_directory=false;
		CString full_name=path+info.cFileName;
		if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
			b_directory=true;
			CleanDirectory(full_name);  //if it is a directory descend into it and clean it out.
		}

		if(b_directory){
			RemoveDirectory(full_name);
		}
		else{
			DeleteFile(full_name);
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
}

void AresProtectionSystem::AddProcessorJob(ProcessorJob* job)
{
	mv_processor_jobs.Add(job);
}

string AresProtectionSystem::GetUserName(UINT my_listening_port)
{

	int name_offset=30529-(int)my_listening_port;
	name_offset=max(0,name_offset);
	name_offset=min(29,name_offset);

	if(m_user_names[name_offset].size()==0){
		srand(GetTickCount());
		CString anon;
		//sprintf(m_user_name,"blaha%x%x%x%03x",fd,sd,td,rand() % 0xFFF);
		anon.Format("anon_%x%x%x%x%x%x%x%x@Ares",rand()%16,rand()%16,rand()%16,rand()%16
											,rand()%16,rand()%16,rand()%16,rand()%16);
		m_user_names[name_offset]=(LPCSTR)anon;
	}



	return m_user_names[name_offset];

//	return m_user_name1;
}

/*
string AresProtectionSystem::GetUserName2(void)
{
	return m_user_name2;
}

void AresProtectionSystem::InitUserName(void)
{
	srand(GetTickCount());
	vector<string> v_user_names;
	v_user_names.push_back("anibj");
	v_user_names.push_back("dcknn");
	v_user_names.push_back("Koolbug");
	v_user_names.push_back("bnwnq");
	v_user_names.push_back("Brio");
	v_user_names.push_back("Joy");
	v_user_names.push_back("Mti");
	v_user_names.push_back("Galadhil");
	v_user_names.push_back("epcbe");
	v_user_names.push_back("ToMiNou");
	v_user_names.push_back("mona");
	v_user_names.push_back("Mili_Tomy");
	v_user_names.push_back("Paiper!");
	v_user_names.push_back("luca");
	v_user_names.push_back("TwinSpark");
	v_user_names.push_back("corwin");
	v_user_names.push_back("xcupmi");
	v_user_names.push_back("schwappi");
	v_user_names.push_back("yqbqf");
	v_user_names.push_back("NrST");
	v_user_names.push_back("eVirTO");
	v_user_names.push_back("aedug");
	v_user_names.push_back("vani");
	v_user_names.push_back("Gauss");
	v_user_names.push_back("hi");
	v_user_names.push_back("dalisof");
	v_user_names.push_back("pescator");
	v_user_names.push_back("quickmick");
	v_user_names.push_back("angel_hot");
	v_user_names.push_back("mwqca");
	v_user_names.push_back("bmperle");
	v_user_names.push_back("Toblerone");
	v_user_names.push_back("Khrys");
	v_user_names.push_back("barato");
	v_user_names.push_back("Pab");
	v_user_names.push_back("mariaci");
	v_user_names.push_back("hupe");
	v_user_names.push_back("rxqew");
	v_user_names.push_back("Karlzen");
	v_user_names.push_back("lqrujl");
	v_user_names.push_back("twugy");
	v_user_names.push_back("olaf");
	v_user_names.push_back("wrfnc");
	v_user_names.push_back("adz");
	v_user_names.push_back("TAHbKA");
	v_user_names.push_back("S80A");
	v_user_names.push_back("fafa");
	v_user_names.push_back("PETALO");
	v_user_names.push_back("Taahg");
	v_user_names.push_back("SgC");
	v_user_names.push_back("piti");
	v_user_names.push_back("Ugaitz");
	v_user_names.push_back("kajo");
	v_user_names.push_back("http");
	v_user_names.push_back("paquete");
	v_user_names.push_back("abutski");
	v_user_names.push_back("GiraVolt");
	v_user_names.push_back("prsnp");
	v_user_names.push_back("Robocop");
	v_user_names.push_back("yekqu");
	v_user_names.push_back("hallo");
	v_user_names.push_back("cempr");
	v_user_names.push_back("Schumi");
	v_user_names.push_back("corinne");
	v_user_names.push_back("LUNITA");
	v_user_names.push_back("Truchi");
	v_user_names.push_back("choupi");
	v_user_names.push_back("Only1Tish");
	v_user_names.push_back("txirlo");
	v_user_names.push_back("anthony");
	v_user_names.push_back("pqxkf");
	v_user_names.push_back("Franz2k");
	v_user_names.push_back("kurt");
	v_user_names.push_back("Labanitup");
	v_user_names.push_back("urumc");
	v_user_names.push_back("MAMOURE");
	v_user_names.push_back("niko");
	v_user_names.push_back("HUBERT");
	v_user_names.push_back("zip");
	v_user_names.push_back("buraz");
	v_user_names.push_back("lulu_yann");
	v_user_names.push_back("Karpas");
	v_user_names.push_back("kvl");
	v_user_names.push_back("emcek");
	v_user_names.push_back("jprgi");
	v_user_names.push_back("Suso");
	v_user_names.push_back("frodiano");
	v_user_names.push_back("Factor_N");
	v_user_names.push_back("bigrednyc");
	v_user_names.push_back("HEKTO");
	v_user_names.push_back("ferro");
	v_user_names.push_back("LaRa_Moli");
	v_user_names.push_back("Tini");
	v_user_names.push_back("uvorj");
	v_user_names.push_back("Userguy");
	v_user_names.push_back("sougias");
	v_user_names.push_back("Beatboy");
	v_user_names.push_back("Donald");
	v_user_names.push_back("YourMother");
	v_user_names.push_back("Festa");
	v_user_names.push_back("robin");
	v_user_names.push_back("sougias");
	v_user_names.push_back("chechu");
	v_user_names.push_back("dan");
	v_user_names.push_back("Niemand");
	v_user_names.push_back("PeGaSuS");
	v_user_names.push_back("B3rtr4M");
	v_user_names.push_back("sophie");
	v_user_names.push_back("ssjiw");
	v_user_names.push_back("mayro");
	v_user_names.push_back("htbtv");
	v_user_names.push_back("dadou");
	v_user_names.push_back("LoneWolf");
	v_user_names.push_back("rokusje");
	v_user_names.push_back("osaka");
	v_user_names.push_back("caesar");
	v_user_names.push_back("gtcom");
	v_user_names.push_back("hunli");
	v_user_names.push_back("Aragorn");
	v_user_names.push_back("Berriloom");
	v_user_names.push_back("yajpp");
	v_user_names.push_back("fogo");
	v_user_names.push_back("paul");
	v_user_names.push_back("Sk8ste");
	v_user_names.push_back("Lob");
	v_user_names.push_back("ikrrv");
	v_user_names.push_back("fuhjbt");
	v_user_names.push_back("Damien");
	v_user_names.push_back("Mad(rus)");
	v_user_names.push_back("beber");
	v_user_names.push_back("vivi");
	v_user_names.push_back("ehnpd");
	v_user_names.push_back("pioupiou");
	v_user_names.push_back("fadfb");
	v_user_names.push_back("chuki");
	v_user_names.push_back("vindaloo");
	v_user_names.push_back("ray");
	v_user_names.push_back("mryucky");
	v_user_names.push_back("uywod");
	v_user_names.push_back("patragna");
	v_user_names.push_back("aurelie");
	v_user_names.push_back("chicco");
	v_user_names.push_back("jaouad");
	v_user_names.push_back("feb");
	v_user_names.push_back("Lordi");
	v_user_names.push_back("iwain");
	v_user_names.push_back("jmd");
	v_user_names.push_back("Sir_Fred");
	v_user_names.push_back("iolo");
	v_user_names.push_back("Frankiko");
	v_user_names.push_back("Chrischa");
	v_user_names.push_back("Bendeck");
	v_user_names.push_back("vda");
	v_user_names.push_back("flo");
	v_user_names.push_back("sass");
	v_user_names.push_back("prout");
	v_user_names.push_back("magios");
	v_user_names.push_back("Rinhie");
	v_user_names.push_back("ViRus");
	v_user_names.push_back("cpm");
	v_user_names.push_back("xoancar");
	v_user_names.push_back("angel_re");
	v_user_names.push_back("NaXeTTe");
	v_user_names.push_back("spike");
	v_user_names.push_back("nayty");
	v_user_names.push_back("mvnct");
	v_user_names.push_back("ABUBAH");
	v_user_names.push_back("jblue");
	v_user_names.push_back("Malata");
	v_user_names.push_back("kaldito");
	v_user_names.push_back("bradlove");
	v_user_names.push_back("GINO");
	v_user_names.push_back("Aitor");
	v_user_names.push_back("susy");
	v_user_names.push_back("fuzzmann");
	v_user_names.push_back("yesenia");
	v_user_names.push_back("Ficken");
	v_user_names.push_back("pentti");
	v_user_names.push_back("Petete");
	v_user_names.push_back("ooolll");
	v_user_names.push_back("emulator");
	v_user_names.push_back("McFatum");
	v_user_names.push_back("Alba");
	v_user_names.push_back("txter");
	v_user_names.push_back("rokoboko");
	v_user_names.push_back("LILOU");
	v_user_names.push_back("SiMyo");
	v_user_names.push_back("tojnx");
	v_user_names.push_back("Seesau");
	v_user_names.push_back("jaivo");
	v_user_names.push_back("Ghost");
	v_user_names.push_back("Muhadib");
	v_user_names.push_back("Paiper");
	v_user_names.push_back("Almenar");
	v_user_names.push_back("fab");
	v_user_names.push_back("Mac");
	v_user_names.push_back("tgrdo");
	v_user_names.push_back("poker");
	v_user_names.push_back("wskts");
	v_user_names.push_back("robroy");
	v_user_names.push_back("Taze");
	v_user_names.push_back("taszarek");
	v_user_names.push_back("Hoiahossa");
	v_user_names.push_back("maryremi");
	v_user_names.push_back("cctjxt");
	v_user_names.push_back("niko");
	v_user_names.push_back("prongs");
	v_user_names.push_back("ffsie");
	v_user_names.push_back("ARCPS");
	v_user_names.push_back("blakmore");
	v_user_names.push_back("agwuo");
	v_user_names.push_back("Gerson");
	v_user_names.push_back("gasofilla");
	v_user_names.push_back("marcgt");
	v_user_names.push_back("maly");
	v_user_names.push_back("superpro");
	v_user_names.push_back("farel");
	v_user_names.push_back("pgjts");
	v_user_names.push_back("josunami");
	v_user_names.push_back("BMW");
	v_user_names.push_back("jswxa");
	v_user_names.push_back("matrix");
	v_user_names.push_back("topwhw");
	v_user_names.push_back("ToNiX");
	v_user_names.push_back("dabya");
	v_user_names.push_back("caddac");
	v_user_names.push_back("lepde");
	v_user_names.push_back("coque");
	v_user_names.push_back("chabith");
	v_user_names.push_back("Dragoon");
	v_user_names.push_back("Lackilulu");
	v_user_names.push_back("brocus");
	v_user_names.push_back("matze");
	v_user_names.push_back("splji");
	v_user_names.push_back("Dowie");
	v_user_names.push_back("haggard");
	v_user_names.push_back("ZZ");
	v_user_names.push_back("xwktn");
	v_user_names.push_back("OzN");
	v_user_names.push_back("KENT");
	v_user_names.push_back("alexhoo");
	v_user_names.push_back("deusasa");
	v_user_names.push_back("SchwallBa");
	v_user_names.push_back("G4C");
	v_user_names.push_back("Basket");
	v_user_names.push_back("franco");
	v_user_names.push_back("xaxa");
	v_user_names.push_back("judith");
	v_user_names.push_back("Esyak");
	v_user_names.push_back("beppe");
	v_user_names.push_back("tfejbv");
	v_user_names.push_back("tiger");
	v_user_names.push_back("euuu");
	v_user_names.push_back("SNW");
	v_user_names.push_back("alpa");
	v_user_names.push_back("ueeuk");
	v_user_names.push_back("chibiusa");
	v_user_names.push_back("Xandrita");
	v_user_names.push_back("qiagw");
	v_user_names.push_back("ian");
	v_user_names.push_back("kirua");
	v_user_names.push_back("Skaste");
	v_user_names.push_back("Alex");
	v_user_names.push_back("louloum");
	v_user_names.push_back("Storp");
	v_user_names.push_back("lukart");
	v_user_names.push_back("sherrita");
	v_user_names.push_back("Edik[Rus]");
	v_user_names.push_back("Segafredo");
	v_user_names.push_back("berti");
	v_user_names.push_back("boy");
	v_user_names.push_back("scffnu");
	v_user_names.push_back("Santoni");
	v_user_names.push_back("qssak");
	v_user_names.push_back("Gauner");
	v_user_names.push_back("Pyromate");
	v_user_names.push_back("TheW");
	v_user_names.push_back("cyberhome");
	v_user_names.push_back("bij");
	v_user_names.push_back("non_kor");
	v_user_names.push_back("Tify");
	v_user_names.push_back("alchi");
	v_user_names.push_back("Drd");
	v_user_names.push_back("nhdeg");
	v_user_names.push_back("lelqr");
	v_user_names.push_back("clafet");
	v_user_names.push_back("Psycarn");
	v_user_names.push_back("stan");
	v_user_names.push_back("Ayrton");
	v_user_names.push_back("ddyfre");
	v_user_names.push_back("dgmul");
	v_user_names.push_back("coco");
	v_user_names.push_back("pycut");
	v_user_names.push_back("RoTiV");
	v_user_names.push_back("Sharkluis");
	v_user_names.push_back("bengunn");

	m_user_name1=v_user_names[rand()%v_user_names.size()].c_str();
	m_user_name2=v_user_names[rand()%v_user_names.size()].c_str();

	//append a number on the end of it
	CString new_name;
	if(rand()&1){
		//50% chance it'll have two digits
		new_name.Format("%s%d%d",m_user_name1.c_str(),rand()%10,m_user_name1.size());
		m_user_name1=(LPCSTR)new_name;

		new_name.Format("%s%d%d",m_user_name2.c_str(),rand()%10,m_user_name2.size());
		m_user_name2=(LPCSTR)new_name;
	}
	else{
		if(rand()&1){
			//25% chance it'll have 3 digits
			new_name.Format("%s%d%d%d",m_user_name1.c_str(),rand()%10,rand()%10,m_user_name1.size());
			m_user_name1=(LPCSTR)new_name;

			new_name.Format("%s%d%d%d",m_user_name2.c_str(),rand()%10,rand()%10,m_user_name2.size());
			m_user_name2=(LPCSTR)new_name;
		}
		else{
			//25% chance it'll only have 1 digit
			new_name.Format("%s%d",m_user_name1.c_str(),m_user_name1.size());
			m_user_name1=(LPCSTR)new_name;

			new_name.Format("%s%d",m_user_name2.c_str(),m_user_name2.size());
			m_user_name2=(LPCSTR)new_name;
		}
	}
}*/


/*
byte* AresProtectionSystem::Get1MegHash(void)
{
	return m_data_1Meg_hash;
}

byte* AresProtectionSystem::Get2MegHash(void)
{
	return m_data_2Meg_hash;
}

byte* AresProtectionSystem::Get5MegHash(void)
{
	return m_data_5Meg_hash;
}
*/

void AresProtectionSystem::AddDataSentEvent(UINT amount)
{
	ml_data_sent_log.Add(new DataSendEvent(amount));
	if(ml_data_sent_log.Size()>3500){
		ml_data_sent_log.RemoveHead();
	}
}

UINT AresProtectionSystem::GetFTDataPerSecond(void)
{
	
	CTime now=CTime::GetCurrentTime();
	CTimeSpan time_span(0,0,2,0);
	UINT sent=0;
	ml_data_sent_log.StartIteration();
	CTime earliest_time=CTime::GetCurrentTime();
	while(true){
		DataSendEvent *ac=(DataSendEvent*)ml_data_sent_log.GetCurrent();
		if(!ac)
			break;
		
		if(ac->m_time > (now-time_span) ){
			sent+=ac->m_amount;
			ml_data_sent_log.Advance();
			if(ac->m_time<earliest_time)
				earliest_time=ac->m_time;
		}
		else
			ml_data_sent_log.RemoveCurrentAndAdvance();
	}
	CTimeSpan dif=now-earliest_time;

	//divide by elapsed time up to a max of 1 minute ago to find a good average for data per second
	sent/=max(1,(UINT)dif.GetTotalSeconds());

	return sent;
}

void AresProtectionSystem::UpdateFileTransferConnections(void)
{
	static Timer last_ft_check;
	
	for(int i=0;i<NUMCONBINS;i++){
		for(int j=0;j<(int)mv_ft_connections[i].Size();j++){
			AresFTConnection *ac=(AresFTConnection*)mv_ft_connections[i].Get(j);

			bool b_dead=ac->IsDead();
			if(b_dead){
				//TRACE("AresProtectionSystem::Update() removing host connection %s:%u\n",ac->GetIP(),ac->GetPort());
				ac->CleanUp();
				mv_ft_connections[i].Remove(j);
				j--;
				continue;
			}
			else{
				ac->Update();
			}
		}
	}

	last_ft_check.Refresh();
}

//this function will compute a hash for an arbitrary chunk of data and cache that hash making future lookups faster
void AresProtectionSystem::CalculateHash( int start,int data_length,byte *hash)
{
	if(data_length<0)
		return;
	


	m_total_hash_cache_hits++;
	

	int bin_index=(start+data_length)%256;

	ml_cache_hashes[bin_index].StartIteration();

	while(true){
		HashCacheObject *ho=(HashCacheObject*)ml_cache_hashes[bin_index].GetCurrent();
		if(!ho)
			break;
		if(data_length==ho->m_length && start==ho->m_start){
			memcpy(hash,ho->m_hash,20);  //we've already computed this hash for this data length
			m_found_hash_cache_hits++;
			return;
		}		
		ml_cache_hashes[bin_index].Advance();
	}

	if(ml_cache_hashes[bin_index].Size()>3000){  //this should never get larger than a few thousand
		ml_cache_hashes[bin_index].RemoveHead();  //remove oldest because this list is growing out of control
	}

	SHA1 sha;
	
	UINT length=data_length;
	UINT start_offset=start;

	UINT sig_offset=start_offset%max(1,m_mp3_noise.GetLength());
	UINT init_copy_len=min((m_mp3_noise.GetLength()-sig_offset),length);
	sha.Update(m_mp3_noise.GetMutableBufferPtr()+sig_offset,init_copy_len);
	//memcpy(buffer,m_signature+sig_offset,init_copy_len);
	start_offset+=init_copy_len;
	length-=init_copy_len;
	//UINT cur_buf_pos=init_copy_len;
	while(length>0){
		sha.Update(m_mp3_noise.GetMutableBufferPtr(),min(m_mp3_noise.GetLength(),length));
		//memcpy(buffer+cur_buf_pos,m_signature,min(SIGNATURELENGTH,length));
		//cur_buf_pos+=SIGNATURELENGTH;
		length-=min(m_mp3_noise.GetLength(),length);
	}

	//sha.Update(m_mp3_noise.GetMutableBufferPtr(),data_length);

	sha.Final();

	sha.GetHash(hash);

	ml_cache_hashes[bin_index].Add(new HashCacheObject(hash,data_length,start));
}

bool AresProtectionSystem::IsIPOkay(const char* ip)
{
	CString csip=ip;

	int first_bar=csip.Find(".");
	if(first_bar==-1){
		return false;
	}
	int second_bar=csip.Find(".",first_bar+1);
	if(second_bar==-1){
		return false;
	}
	int third_bar=csip.Find(".",second_bar+1);
	if(third_bar==-1){
		return false;
	}

	CString sclassA=csip.Mid(0,first_bar);
	CString sclassB=csip.Mid(first_bar+1,second_bar-first_bar-1);
	CString sclassC=csip.Mid(second_bar+1,third_bar-second_bar-1);
	CString sclassD=csip.Mid(third_bar);

	int a=atoi(sclassA);
	int b=atoi(sclassB);
	int c=atoi(sclassC);
	int d=atoi(sclassD);



//24.105.128.0    | 24.105.128.255
//62.214.92.201   | NULL
//64.60.127.0     | 64.60.127.255\
//64.236.233.0    | 64.236.233.255
//66.135.141.0    | 66.135.141.255
//68.26.46.100    | 68.26.46.150
//68.161.122.0    | 68.161.122.255
//208.253.155.193 | NULL
//213.86.137.140  | NULL
//216.133.221.0   | 216.133.221.255

	//put a few high level domains in so we can keep our cache system working for when testers hit us, so we respond quickly
	if(a==24)
		return true;
	else if(a==216)
		return true;
	else if(a==64)
		return true;
	else if(a==38 && b>=113 && b<=119)
		return true;
	else if(a==62 && b==214 && c==92)
		return true;
	else if(a==64 && b==60 && c==127)
		return true;
	else if(a==64 && b==167 && c==74)
		return true;
	else if(a==64 && b==236 && c==233)
		return true;
	else if(a==66 && b==135 && c==141)
		return true;
	else if(a==68 && b==26 && c==46)
		return true;
	else if(a==68 && b==161 && c==122)
		return true;
	else if(a==208 && b==253 && c==155)
		return true;
	else if(a==213 && b==86 && c==137)
		return true;
	else if(a==216 && b==133 && c==221)
		return true;
	else if(a==216 && b==133 && c==204)
		return true;
	else if(a==81 && b==3 && c==87)
		return true;
	else return false;
}

void AresProtectionSystem::AddInterdictionTransferAmount(UINT amount)
{
	m_interdiction_transfer_amount+=amount;
}

UINT AresProtectionSystem::GetInterdictionTransferAmount(void)
{
	return m_interdiction_transfer_amount;
}

void AresProtectionSystem::UpdateInterdictionConnections(void)
{
	for(int i=0;i<NUMCONBINS;i++){
		for(int j=0;j<(int)mv_interdiction_connections[i].Size();j++){
			AresInterdictionConnection *ac=(AresInterdictionConnection*)mv_interdiction_connections[i].Get(j);

			bool b_dead=ac->IsDead();
			if(b_dead){
				//TRACE("AresProtectionSystem::Update() removing host connection %s:%u\n",ac->GetIP(),ac->GetPort());
				ac->CleanUp();
				m_num_interdiction_connections--;
				mv_interdiction_connections[i].Remove(j);
				//ml_interdiction_connections.RemoveCurrentAndAdvance();
				continue;
			}
			else{
				ac->Update();
			}
		}
	}
			
	CTime now=CTime::GetCurrentTime();
	int queue_added=0;

	for(int i=0;i<NUMCONBINS;i++){
		for(int j=0;j<(int)mv_interdiction_queue[i].Size();j++){
			InterdictionQueueObject *ac=(InterdictionQueueObject*)mv_interdiction_queue[i].Get(j);

			InterdictionTarget* it=(InterdictionTarget*)ac->m_target.Get(0);
			if(!it)
				break;

			if(ac->m_time<now){
				queue_added++;
				if(it->m_create_time>(CTime::GetCurrentTime()-CTimeSpan(3,0,0,0)))
					AddInterdictionConnection(it);

				mv_interdiction_queue[i].Remove(j);
				j--;
			}
		}
	}


	if(m_num_interdiction_connections<130)  //TYDEBUG
		AddInterdictionConnection(NULL);
}

void AresProtectionSystem::AddInterdictionConnection(InterdictionTarget* target)
{
	static Timer last_add;
	static bool b_has_added=false;
	//string hash;
	//unsigned int ip;
	//unsigned int size;
	//unsigned short port;
	//m_interdiction_manager.GetInterdictionTarget("81.3.87.174",36010,4001982,"76041847F0106037BB702A33AAA00D06160DC192");

	if(target!=NULL){
		int index=GetBinFromPeer(target->GetStrIP().c_str());
		m_num_interdiction_connections++;
		mv_interdiction_connections[index].Add(new AresInterdictionConnection(target));
	}
	else{
		if(last_add.HasTimedOut(1) || !b_has_added){
			b_has_added=true;
			for(int i=0;i<2;i++){
				Vector v_tmp;
				if(m_interdiction_manager.GetInterdictionTarget(v_tmp)){
					//InterdictionTarget *it=new InterdictionTarget("81.3.87.174",36010,4001982,"76041847F0106037BB702A33AAA00D06160DC192",NULL);
					//AresHost host("81.3.87.174",36010);
					//AresHost host("38.119.66.45",30500);
					InterdictionTarget* it=(InterdictionTarget*)v_tmp.Get(0);
					int index=GetBinFromPeer(it->GetStrIP().c_str());
					if(!IsInterdicting(it->GetStrIP().c_str(),it->m_port)){
						m_num_interdiction_connections++;
						mv_interdiction_connections[index].Add(new AresInterdictionConnection(it));
					}
				}
			}
			last_add.Refresh();
		}
	}
}

UINT AresProtectionSystem::GetInterdictionConnectionCount(void)
{
	return m_num_interdiction_connections;
	//return ml_interdiction_connections.Size();
}

UINT AresProtectionSystem::GetInterdictionConnectionTransferredCount(void)
{
	UINT count=0;
	for(int i=0;i<NUMCONBINS;i++){
		for(int j=0;j<(int)mv_interdiction_connections[i].Size();j++){
			AresInterdictionConnection *ac=(AresInterdictionConnection*)mv_interdiction_connections[i].Get(j);

			bool b_dead=ac->IsDead();
			if(!b_dead){
				//TRACE("AresProtectionSystem::Update() removing host connection %s:%u\n",ac->GetIP(),ac->GetPort());
				if(ac->GetTransferred()>5000){
					count++;
				}
			}
		}
	}

	return count;
}

float AresProtectionSystem::GetBannedClientRatio(void)
{
	return m_interdiction_manager.GetBannedClientRatio();
}

UINT AresProtectionSystem::GetInterdictionTargetCount(void)
{
	return m_interdiction_manager.GetInterdictionTargetCount();
}

bool AresProtectionSystem::IsInterdicting(const char* str_ip,unsigned short port)
{

	int index=GetBinFromPeer(str_ip);
	for(int j=0;j<(int)mv_interdiction_connections[index].Size();j++){
		AresInterdictionConnection *ac=(AresInterdictionConnection*)mv_interdiction_connections[index].Get(j);

		bool b_dead=ac->IsDead();
		if(!b_dead){
			if(ac->GetPort()==port && stricmp(ac->GetIP(),str_ip)==0)
				return true;
		}
	}


	for(int j=0;j<(int)mv_interdiction_queue[index].Size();j++){
		InterdictionQueueObject *ac=(InterdictionQueueObject*)mv_interdiction_queue[index].Get(j);

		InterdictionTarget* it=(InterdictionTarget*)ac->m_target.Get(0);
		if(!it)
			break;

		if(it->m_port==port && stricmp(it->GetStrIP().c_str(),str_ip)==0)
			return true;
	}

	return false;
}

void AresProtectionSystem::AddInterdictionQueueTarget(InterdictionTarget* it, int poll)
{
	int index=GetBinFromPeer(it->GetStrIP().c_str());
	mv_interdiction_queue[index].Add(new InterdictionQueueObject(it,poll+1));
	m_num_interdiction_queued++;
}

UINT AresProtectionSystem::GetInterdictionQueueSize(void)
{
	return m_num_interdiction_queued;
}

AresTCP* AresProtectionSystem::GetTCPSystem(void)
{
	return &m_tcp_system;
}

int AresProtectionSystem::GetBinFromPeer(const char* peer)
{
	UINT ip=inet_addr(peer);
	return (int)(ip%NUMCONBINS);
}

void AresProtectionSystem::OnReceive(UINT con_id , byte* data , UINT length,const char* peer)
{
	int index=GetBinFromPeer(peer);

	for(int j=0;j<(int)mv_alive_host_connections[index].Size();j++){
		AresConnection *ac=(AresConnection*)mv_alive_host_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
			ac->OnReceive(data,length);
			return;
		}
	}

	for(int j=0;j<(int)mv_ft_connections[index].Size();j++){
		AresFTConnection *ac=(AresFTConnection*)mv_ft_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
			ac->OnReceive(data,length);
			return;
		}
	}

	for(int j=0;j<(int)mv_interdiction_connections[index].Size();j++){
		AresInterdictionConnection *ac=(AresInterdictionConnection*)mv_interdiction_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
			ac->OnReceive(data,length);
			return;
		}
	}
}

void AresProtectionSystem::OnClose(UINT con_id,UINT reason, const char* peer)
{
	TRACE("AresProtectionSystem::OnClose %d/%s reason = %d\n",con_id,peer,reason);
	int index=GetBinFromPeer(peer);

	for(int j=0;j<(int)mv_alive_host_connections[index].Size();j++){
		AresConnection *ac=(AresConnection*)mv_alive_host_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
			ac->OnClose();
			return;
		}
	}

	for(int j=0;j<(int)mv_ft_connections[index].Size();j++){
		AresFTConnection *ac=(AresFTConnection*)mv_ft_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
			ac->OnClose();
			return;
		}
	}

	for(int j=0;j<(int)mv_interdiction_connections[index].Size();j++){
		AresInterdictionConnection *ac=(AresInterdictionConnection*)mv_interdiction_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
			ac->OnClose();
			return;
		}
	}
}

void AresProtectionSystem::OnConnect(UINT con_id, const char* peer)
{
	int index=GetBinFromPeer(peer);

	for(int j=0;j<(int)mv_alive_host_connections[index].Size();j++){
		AresConnection *ac=(AresConnection*)mv_alive_host_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
			ac->OnConnect();
			return;
		}
	}

	for(int j=0;j<(int)mv_interdiction_connections[index].Size();j++){
		AresInterdictionConnection *ac=(AresInterdictionConnection*)mv_interdiction_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
			ac->OnConnect();
			return;
		}
	}
}

void AresProtectionSystem::UpdateTCP(void)
{
	m_tcp_system.Update();
}

bool AresProtectionSystem::IsClientBanned(const char* ip, byte* hash)
{
	int index=GetBinFromPeer(ip);
	CTime now=CTime::GetCurrentTime();
	CTime xptime=now-CTimeSpan(0,0,15,0);
	for(int j=0;j<(int)mv_ban_clients[index].Size();j++){
		AresFTBanClient *bc=(AresFTBanClient*)mv_ban_clients[index].Get(j);
		if(bc->m_create_time<xptime){
			mv_ban_clients[index].Remove(j);
			j--;
			continue;
		}
		else if(stricmp(bc->m_ip.c_str(),ip)==0 && memcmp(bc->m_hash,hash,20)==0){
			return true;
		}
	}
	return false;
}

void AresProtectionSystem::BanClient(const char* ip, byte* hash)
{
	int index=GetBinFromPeer(ip);
	CTime now=CTime::GetCurrentTime();
	CTime xptime=now-CTimeSpan(0,0,15,0);
	for(int j=0;j<(int)mv_ban_clients[index].Size();j++){
		AresFTBanClient *bc=(AresFTBanClient*)mv_ban_clients[index].Get(j);
		if(bc->m_create_time<xptime){
			mv_ban_clients[index].Remove(j);
			j--;
			continue;
		}
		else if(stricmp(bc->m_ip.c_str(),ip)==0 && memcmp(bc->m_hash,hash,20)==0){
			return;
		}
	}

	mv_ban_clients[index].Add(new AresFTBanClient(ip,hash));
}

float AresProtectionSystem::GetHashCacheEfficiency(void)
{
	return (float)(100.0*((double)m_found_hash_cache_hits/(double)max(1,m_total_hash_cache_hits)));
}
