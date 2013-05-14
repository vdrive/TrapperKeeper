#include "StdAfx.h"
#include "aressupernodesystem.h"
#include "AresSNConnection.h"
#include "AresSNClientConnection.h"
#include "..\tkcom\ServerSocket.h"


//static variable initialization
UINT AresSupernodeSystem::sm_system_count=0;
AresSupernodeSystem* AresSupernodeSystem::sm_system=NULL;
CCriticalSection AresSupernodeSystem::sm_system_lock;

HANDLE sm_log_file_handle=NULL;
UINT sm_log_bytes_written=0;
CCriticalSection sm_log_lock;


AresSupernodeSystem::AresSupernodeSystem(void)
{
	mb_processor_ready=false;
	m_num_host_connections=0;
	m_forward_index1=0;
	m_forward_index2=0;
}

AresSupernodeSystem::~AresSupernodeSystem(void)
{
	if(mp_processor!=NULL)
		delete mp_processor;
	mp_processor=NULL;

	if(sm_log_file_handle!=NULL){
		CloseHandle(sm_log_file_handle);
		sm_log_file_handle=NULL;
	}
}

void AresSupernodeSystem::StartSystem(void)
{
	srand(GetTickCount());
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

	InitUserName();

	m_spoof_cache.StartThreadLowPriority();
	m_host_cache.StartSystem();
	this->StartThreadLowPriority();
	m_udp_dispatcher.StartThreadLowPriority();

	LogQuery("Query log running...");
	LogConnectionInfo("Connection log running...");

	ServerSocket *ns=new ServerSocket();
	ns->Init(BASEPORT,this);
	mv_server_sockets.Add(ns);

	m_decoy_cache.StartThreadLowPriority();
}

void AresSupernodeSystem::StopSystem(void)
{
	m_decoy_cache.b_killThread=1;
	for(int i=0;i<(int)mv_server_sockets.Size();i++){
		ServerSocket *ss=(ServerSocket*)mv_server_sockets.Get(i);
		ss->Close();
	}

	m_spoof_cache.StopThread();
	m_host_cache.StopSystem();
	this->StopThread();
	m_udp_system.StopSystem();
	m_udp_dispatcher.StopThread();
	m_decoy_cache.StopThread();
}

UINT AresSupernodeSystem::Run(){
	int delay=min(LOADDELAY,600);
	for(int i=0;i<delay && !this->b_killThread;i++){ //sleep for X seconds before we start the processor (in case the assembly.bin file hasn't quite finished synching yet or its causing it to crash for some reason, at least we'll be able to patch before TK crashes)
		Sleep(1000);  
	}
	if(!this->b_killThread){
		LogConnectionInfo("Loading Ares virtual machine...");
		mp_processor=new Processor();
		LogConnectionInfo("Finished loading Ares virtual machine.");
		mb_processor_ready=true;
	}
	
	
	while((!m_spoof_cache.IsReady() || !m_host_cache.IsReady()) && !this->b_killThread){
		Sleep(200);
	}

	if(this->b_killThread)
		return 0;

	//start the udp system, since the processor is now ready to serve it
	m_udp_system.StartSystem();

	while(!this->b_killThread){
		Sleep(25);
		m_udp_system.Update();
		if(mv_processor_jobs.Size()>0){
			LogToFile("AresSupernodeSystem::Run() BEGIN - Processing Job");
//			Log("Ares Processor:  Started processing 0x38 message....");
			ProcessorJob* job=(ProcessorJob*)mv_processor_jobs.Get(0);
			DWORD time1=GetTickCount();
			mp_processor->ProcessSNData(job->m_eighty_in,job->m_twenty_out);

			DWORD time2=GetTickCount();
			job->mb_done=true;  //signal this job as having been completed

			mv_processor_jobs.Remove(0);

			double dtime1=(double)time1;
			double dtime2=(double)time2;

			double seconds=(dtime2-dtime1)/1000.0;

			CString log_msg;
			log_msg.Format("Ares Processor:  Did ares virtual machine run in %.2f seconds!",(float)seconds);
			LogConnectionInfo(log_msg);
			LogToFile("AresSupernodeSystem::Run() END - Processing Job");
		}
	}
	return 0;
}

void AresSupernodeSystem::OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection)
{
//	delete new_connection;
	PeerSocket* ps=(PeerSocket*)new_connection;
	TRACE("AresSupernodeSystem::OnAccept() %s\n",ps->GetIP());
	int bin_index=GetBinFromPeer(ps->GetIP());

	mv_client_connections[bin_index].Add(new AresSNClientConnection(ps));
}

AresSNSpoofCache* AresSupernodeSystem::GetSpoofCache(void)
{
	return &m_spoof_cache;
}

void AresSupernodeSystem::OnReceive(UINT con_id , byte* data , UINT length,const char* peer){
	int index=GetBinFromPeer(peer);

	for(int j=0;j<(int)mv_sn_connections[index].Size();j++){
		AresSNConnection *ac=(AresSNConnection*)mv_sn_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
#ifdef TKSOCKETSYSTEM
			ac->OnReceive(data,length);
#endif
			return;
		}
	}

}

void AresSupernodeSystem::OnClose(UINT con_id,UINT reason, const char* peer){
	int index=GetBinFromPeer(peer);

	for(int j=0;j<(int)mv_sn_connections[index].Size();j++){
		AresSNConnection *ac=(AresSNConnection*)mv_sn_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
#ifdef TKSOCKETSYSTEM
			ac->OnClose();
#endif
			return;
		}
	}

}

void AresSupernodeSystem::OnConnect(UINT con_id, const char* peer){
	int index=GetBinFromPeer(peer);

	for(int j=0;j<(int)mv_sn_connections[index].Size();j++){
		AresSNConnection *ac=(AresSNConnection*)mv_sn_connections[index].Get(j);
		if(ac->GetConHandle()==con_id){
#ifdef TKSOCKETSYSTEM
			ac->OnConnect();
#endif
			return;
		}
	}

}

#ifdef TKSOCKETSYSTEM
TKSocketSystem* AresSupernodeSystem::GetTCPSystem(void)
{
	return &m_tcp_system;
}
#endif

bool AresSupernodeSystem::IsConnectedToUltraPeer(const char* host_ip)
{
	int index=GetBinFromPeer(host_ip);

	for(int i=0;i<(int)mv_sn_connections[index].Size();i++){
		AresSNConnection *ac=(AresSNConnection*)mv_sn_connections[index].Get(i);

		if(!ac->IsDying() && stricmp(ac->GetIP(),host_ip)==0)
			return true;
	}

	return false;
}

void AresSupernodeSystem::UpdateTCP(void)
{
#ifdef TKSOCKETSYSTEM
	m_tcp_system.Update();
#endif
}

const char* AresSupernodeSystem::GetSystemIP(void)
{
	if(mv_system_ips.size()>0){
		return mv_system_ips[0].c_str();
	}
	else return "5.6.7.8";
}

void AresSupernodeSystem::InitUserName(void)
{
	
	for(int i=0;i<7;i++){
		char buf[2];
		buf[0]='a'+rand()%26;
		buf[1]=(char)0;
		m_user_name+=buf;
	}
}

const char* AresSupernodeSystem::GetUserName(void)
{
	return m_user_name.c_str();
}

UINT AresSupernodeSystem::GetEventCacheTime(void)
{
	return 0;
}

void AresSupernodeSystem::AddProcessorJob(ProcessorJob* job)
{
	mv_processor_jobs.Add(job);
}

void AresSupernodeSystem::Update(void)
{
	LogToFile("AresSupernodeSystem::Update() BEGIN");

	if(!mb_processor_ready || !m_spoof_cache.IsReady() || !m_host_cache.IsReady()){
		LogToFile("AresSupernodeSystem::Update() END 0");
		return;  //nothing to update and we don't want to make any new connections yet
	}

	LogToFile("AresSupernodeSystem::Update() Position A");

	static Timer last_update;
	static Timer last_update2;

	//see if there are any clients ready to be removed from our master list, update the other clients
	if(last_update.HasTimedOut(5)){
		last_update.Refresh();

		for(int i=0;i<NUMCONBINS;i++){
			for(int j=0;j<(int)mv_sn_connections[i].Size();j++){
				AresSNConnection *ac=(AresSNConnection*)mv_sn_connections[i].Get(j);
				
				if(!ac->IsDying()){
					ac->Update();  //see if it is dead yet
				}
				else{
					//TRACE("AresProtectionSystem::Update() removing host connection %s:%u\n",ac->GetIP(),ac->GetPort());
					ac->CleanUp();
					mv_sn_connections[i].Remove(j);
					m_num_host_connections--;
					j--;
					continue;
				}
			}
		}
	}

	if(last_update2.HasTimedOut(6)){
		last_update2.Refresh();

		for(int i=0;i<NUMCONBINS;i++){
			for(int j=0;j<(int)mv_client_connections[i].Size();j++){
				AresSNClientConnection *ac=(AresSNClientConnection*)mv_client_connections[i].Get(j);
				
				if(!ac->IsDying()){
					ac->Update();  //see if it is dead yet
				}
				else{
					//TRACE("AresProtectionSystem::Update() removing host connection %s:%u\n",ac->GetIP(),ac->GetPort());
					ac->CleanUp();
					mv_client_connections[i].Remove(j);
					j--;
					continue;
				}
			}
		}
	}

	//initiated new connections
	//  TYDEBUG
	
	LogToFile("AresSupernodeSystem::Update() Position B");

	//TYDEBUG
	static bool b_reached_quota=false;
//	if(m_udp_system.GetAliveHostCount()>=300)
//		b_reached_quota=true;

	bool b_using_tcp=true;  //toggle for testing with and without TCP/IP

	UINT tcp_connection_limit=1600;
	UINT udp_connection_limit=400;

	for(int i=0;i<15 && !b_reached_quota && mv_processor_jobs.Size()<15 && (m_num_host_connections<tcp_connection_limit || m_udp_system.GetAliveHostCountOutbound()<udp_connection_limit);i++){  //open X new connections per update

		if(!b_using_tcp && m_udp_system.GetAliveHostCountOutbound()>=udp_connection_limit){
			break;
		}

		Vector v_tmp;
		if(m_num_host_connections<tcp_connection_limit && b_using_tcp)
			m_host_cache.GetNextHost(v_tmp,true);
		else
			m_host_cache.GetNextHost(v_tmp,false);

		if(v_tmp.Size()>0){
			
			//room to add more udp contacts
			AresHost* host=(AresHost*)v_tmp.Get(0);

			bool b_added=false;

			if(m_udp_system.GetAliveHostCountOutbound()<udp_connection_limit){
				m_udp_system.AddHost(host->GetIP(),host->GetPort(),false);
				b_added=true;
			}
			if(m_num_host_connections<tcp_connection_limit && b_using_tcp){
				//TRACE("Testing %s connectivity\n",host->GetIP());
				if(!IsConnectedToUltraPeer(host->GetIP())){
					//TRACE("Attempting connection to %s\n",host->GetIP());
					//giFT = port 2597
					//gnutella = port 6346
					//we are not already connected to this ultra peer so lets open a connection

					int index=GetBinFromPeer(host->GetIP());
					AresSNConnection *new_con=new AresSNConnection(host,30500+rand()%30);
					mv_sn_connections[index].Add(new_con);
					m_num_host_connections++;
					new_con->Connect();
				}
				b_added=true;
			}
			if(!b_added){
				break;
			}
		}
	}

	LogToFile("AresSupernodeSystem::Update() END");
}

UINT AresSupernodeSystem::GetTotalConnections(void)
{
	return m_num_host_connections;
}

UINT AresSupernodeSystem::GetFullyConnected(void)
{
	UINT count=0;
	for(int i=0;i<NUMCONBINS;i++){
		for(int j=0;j<(int)mv_sn_connections[i].Size();j++){
			AresSNConnection *ac=(AresSNConnection*)mv_sn_connections[i].Get(j);
			
				//TRACE("AresProtectionSystem::Update() removing host connection %s:%u\n",ac->GetIP(),ac->GetPort());
			if(!ac->IsDying() && ac->IsFullyConnected())
				count++;
		}
	}
	return count;
}

UINT AresSupernodeSystem::GetKnownHosts(void)
{
	return m_host_cache.GetKnownHosts();
}

void AresSupernodeSystem::LogQuery(const char* str)
{
	//TRACE("Log query: %s\n",str);
	static query_entry_num=0;
	if(mv_query_log.Size()<50)
		mv_query_log.Add(new AresSNLogEntry(str,query_entry_num++));
}

void AresSupernodeSystem::LogConnectionInfo(const char* str)
{
	//TRACE("Log connection info: %s\n",str);
	static connection_entry_num=0;
	if(mv_connection_log.Size()<50)
		mv_connection_log.Add(new AresSNLogEntry(str,connection_entry_num++));
}

void AresSupernodeSystem::PopQueryLog(Vector& v)
{
	if(mv_query_log.Size()>0){
		v.Add(mv_query_log.Get(0));
		mv_query_log.Remove(0);
	}
}

void AresSupernodeSystem::PopConnectionLog(Vector& v)
{
	if(mv_connection_log.Size()>0){
		Object *o=mv_connection_log.Get(0);
		v.Add(o);
		mv_connection_log.Remove(0);
	}
}

UINT AresSupernodeSystem::GetLoadedDecoys(void)
{
	return m_spoof_cache.GetLoadedDecoys();
}

void AresSupernodeSystem::LogToFile(const char* str)
{
	//maintain synchronous access to this function
/*
	CSingleLock lock(&sm_log_lock,TRUE);

	if(sm_log_bytes_written>6000000){  //every X amount of data, reset the log to prevent uncontrollable growth.
		CloseHandle(sm_log_file_handle);
		sm_log_file_handle=NULL;
		sm_log_bytes_written=0;
	}
	if(sm_log_file_handle==NULL){
		CreateDirectory("c:\\AresSupernode",NULL);
		CreateDirectory("c:\\AresSupernode\\Logs",NULL);
		sm_log_file_handle = CreateFile("c:\\AresSupernode\\Logs\\debug_log.txt",			// open file at local_path 
				GENERIC_WRITE,              // open for writing 
				FILE_SHARE_READ,							// we don't like to share
				NULL,						// no security 
				CREATE_ALWAYS,				// create new whether file exists or not
				FILE_ATTRIBUTE_NORMAL,		// normal file 
				NULL);						// no attr. template 
	}
	CString output;
	CTime cur_time=CTime::GetCurrentTime();
	output.Format("%s:  %s\r\n",cur_time.Format("%A %I:%M %p"),str);
	DWORD tmp;
	sm_log_bytes_written+=output.GetLength();
	WriteFile(sm_log_file_handle,output.GetBuffer(output.GetLength()),output.GetLength(),&tmp,NULL);
	*/
}

UINT AresSupernodeSystem::GetUDPHostCount(void)
{
	return m_udp_system.GetAliveHostCount();
}

void AresSupernodeSystem::SaveHost(const char* host_ip , unsigned short port)
{
	m_host_cache.SaveHost(host_ip,port);
}

AresSNHostCache* AresSupernodeSystem::GetHostCache(void)
{
	return &m_host_cache;
}

AresSupernodeUDPDispatcher* AresSupernodeSystem::GetUDPDispatcher(void)
{
	return &m_udp_dispatcher;
}

Buffer2000* AresSupernodeSystem::GetGUIDBuffer(void)
{
	if(m_guid_buffer.GetLength()==0){
		// Create and copy 8 16-byte GUIDs to the end of the data
		for(int i=0;i<8;i++)
		{
			byte guid[sizeof(GUID)];
			CoCreateGuid((GUID*)&guid);  //give us a random 16 byte array basically, WARNING though this may be useable to track our machines not sure since i believe this incorporates MAC address
			m_guid_buffer.WriteBytes(guid,sizeof(GUID));
		}
	}
	return &m_guid_buffer;
}

void AresSupernodeSystem::ForwardTCPQuery(Buffer2000* query)
{
	for(int forward_index=0;forward_index<1 && forward_index<(int)m_num_host_connections;forward_index++){
		if(m_forward_index1>=NUMCONBINS)
			m_forward_index1=0;

		if(m_forward_index2>=mv_sn_connections[m_forward_index1].Size()){
			m_forward_index2=0;
			m_forward_index1++;  //advance our bin index by 1
		}
		else{
			AresSNConnection* tmp_host=(AresSNConnection*)mv_sn_connections[m_forward_index1].Get(m_forward_index2++);
			if(!tmp_host->IsDead()){
				tmp_host->ForwardQuery(query);
			}
		}
	}
}

AresDecoyPoolCache* AresSupernodeSystem::GetDecoyCache(void)
{
	return &m_decoy_cache;
}
