#include "StdAfx.h"
#include "aresdatacollectorsystem.h"
#include "..\tkcom\Timer.h"
#include "AresDCConnection.h"

UINT AresDataCollectorSystem::sm_system_count=0;
AresDataCollectorSystem* AresDataCollectorSystem::sm_system=NULL;
CCriticalSection AresDataCollectorSystem::sm_system_lock;
AresDataCollectorSystem AresDataCollectorSystemReference::mp_system;

AresDataCollectorSystem::AresDataCollectorSystem(void)
{
	mb_force_connection_purge=false;
	m_connecting_sockets=0;
	mp_processor=NULL;
	mb_processor_ready=false;
	mb_controller=false;
}

AresDataCollectorSystem::~AresDataCollectorSystem(void)
{
	if(mp_processor){
		delete mp_processor;
		mp_processor=NULL;
	}
}

UINT AresDataCollectorSystem::Run(void)
{
	int sleep_time=rand()%600;
	if(mb_controller)
		sleep_time=0;
	for(int i=0;i<sleep_time*10 && !this->b_killThread;i++){  //sleep for random time between 0 and 10 minutes before we wake up
		Sleep(100);
	}
	m_project_info_cache.StartThread();
	m_host_cache.StartThread();

	if(mb_controller){
		//TYDEBUG - disabled because it appears we don't really need interdiction
		//m_target_manager.StartThread();
		//m_interdiction_server.StartSystem();
	}

	Log("Ares System:  Initializing processor...");
	mp_processor=new Processor();
	Log("Ares System:  Processor inited and ready.");

	mb_processor_ready=true;
	while(!this->b_killThread){
		Sleep(50);
		if(mv_processor_jobs.Size()>0){
			Log("Ares Processor:  Started processing 0x38 message....");
			ProcessorJob* job=(ProcessorJob*)mv_processor_jobs.Get(0);
			DWORD time1=GetTickCount();
			mp_processor->ProcessData(job->m_sixteen_in,job->m_twenty_out);

			DWORD time2=GetTickCount();
			job->mb_done=true;  //signal this job as having been completed

			mv_processor_jobs.Remove(0);

			double dtime1=(double)time1;
			double dtime2=(double)time2;

			double seconds=(dtime2-dtime1)/1000.0;

			CString log_msg;
			log_msg.Format("Ares Processor:  Finished processing 0x38 message in %.2f seconds!",(float)seconds);
			Log(log_msg);
		}
	}
	return 0;
}

void AresDataCollectorSystem::StartSystem(void)
{
	OFSTRUCT of;
	ZeroMemory(&of,sizeof(of));
	if( OpenFile( "c:\\syncher\\ares_controller.txt" , &of , OF_EXIST ) != HFILE_ERROR ){
		mb_controller=true;  //the file exists, we should be the controller
	}

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
				
				CString tmp_str;
				tmp_str.Format("AresDataCollector System IP #%d: %s\n", ipCount, mv_system_ips[ipCount].c_str());
				Log(tmp_str);
				TRACE("AresDataCollectorSystem IP #%d: %s\n", ipCount, mv_system_ips[ipCount].c_str());
				++ipCount;
			}
		}
	}



	this->StartThreadLowPriority();

	if(IsController()){
		m_supernode_server_system.StartSystem();
	}
	m_supernode_server_system_interface.StartSystem();
	
	m_server_socket.Init(ARESDCSERVERPORT,this);
	
}

void AresDataCollectorSystem::StopSystem(void)
{
	m_target_manager.b_killThread=1;
	m_host_cache.StopThread();
	m_project_info_cache.StopThread();
	this->StopThread();
	m_target_manager.StopThread();

	m_interdiction_server.StopSystem();

	ml_host_connections.StartIteration();
	while(true){
		AresDCConnection *ac=(AresDCConnection*)ml_host_connections.GetCurrent();
		if(!ac)
			break;
		
		ac->CleanUp();
		ml_host_connections.RemoveCurrentAndAdvance();
	}

	ml_alive_host_connections.Clear();

	m_server_socket.Close();

	m_supernode_server_system.StopSystem();
	m_supernode_server_system_interface.StopSystem();

}

void AresDataCollectorSystem::UpdateConnecting(int dif)
{
	m_connecting_sockets+=dif;
}

UINT AresDataCollectorSystem::GetEventCacheTime(void)
{
	return 0;
}

AresDCHostCache* AresDataCollectorSystem::GetHostCache(void)
{
	return &m_host_cache;
}

const char* AresDataCollectorSystem::GetSystemIP(void)
{
	if(mv_system_ips.size()>0){
		return mv_system_ips[0].c_str();
	}
	else return "1.2.3.4";
}

void AresDataCollectorSystem::OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection)
{
	delete new_connection;
}

bool AresDataCollectorSystem::IsConnectedToUltraPeer(const char* host_ip)
{
	ml_alive_host_connections.StartIteration();
	while(true){
		AresDCConnection *ac=(AresDCConnection*)ml_alive_host_connections.GetCurrent();
		if(!ac)
			return false;
		
		if(stricmp(ac->GetIP(),host_ip)==0){
			return true;
		}

		ml_alive_host_connections.Advance();
	}

	return false;
}

void AresDataCollectorSystem::Update(void)
{
	if(!m_project_info_cache.IsReady() || !mb_processor_ready)
		return;

	if(ml_host_connections.Size()==0)  //only first time through do we set this
		m_status="Operational - Uploading Files To Hosts";

	bool b_can_clean=false;

	if(ml_host_connections.Size()>1350 || m_connection_purge_timer.HasTimedOut(15) || m_connection_purge_timer.HasTimedOut(60*30) || mb_force_connection_purge){
		b_can_clean=true;
		mb_force_connection_purge=false;
		m_connection_purge_timer.Refresh();
	}

	static Timer last_dying_update;

	//see if there are any clients ready to be removed from our master linked list
	if(last_dying_update.HasTimedOut(5) || b_can_clean){
		last_dying_update.Refresh();
		ml_host_connections.StartIteration();
		while(true){
			AresDCConnection *ac=(AresDCConnection*)ml_host_connections.GetCurrent();
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
		}
	}

	static Timer last_living_update;

	//give special considering to client connection objects that are not disconnected
	if(last_living_update.HasTimedOut(3) || b_can_clean){

		bool b_search=false;
		Vector v_search_project;
		if(m_last_search.HasTimedOut(60)){
			m_last_search.Refresh();
			b_search=true;
			m_project_info_cache.GetNextProjectForSearch(v_search_project);
		}


		last_living_update.Refresh();

		ml_alive_host_connections.StartIteration();
		while(true){
			AresDCConnection *ac=(AresDCConnection*)ml_alive_host_connections.GetCurrent();
			if(!ac)
				break;
			
			if(ac->IsDying()){
				ml_alive_host_connections.RemoveCurrentAndAdvance();  //never check this one again
				continue;
			}
			else{
				if(b_search && v_search_project.Size()>0){
					ac->SearchProject((AresDCProjectInfo*)v_search_project.Get(0));
				}

				ac->Update();
				if(b_can_clean)
					ac->PurgeEvents();
			}

			ml_alive_host_connections.Advance();
		}
	}

	//initiated new connections
	int limit=35;   //TYDEBUG
	if(ml_alive_host_connections.Size()>700)
		limit=25;  //gets to be higher cpu because of our inefficient linear search coming up

	for(int i=0;i<limit && m_connecting_sockets<8 && mv_processor_jobs.Size()<4;i++){  //open X new connections per update
		if(ml_alive_host_connections.Size()<1200){
			AresHost *host=m_host_cache.GetNextHost();
			if(host){
				if(!IsConnectedToUltraPeer(host->GetIP())){
					//CString tmp;
					//tmp.Format("Creating Connection for %s, %d connecting sockets",host->GetIP(),m_connecting_sockets);
					//Log(tmp);

					//giFT = port 2597
					//gnutella = port 6346
					//we are not already connected to this ultra peer so lets open a connection
					AresDCConnection *new_con=new AresDCConnection(host,30500+rand()%30);
					ml_host_connections.Add(new_con);
					ml_alive_host_connections.Add(new_con);
					new_con->Connect();
				}
			}
		}
	}
}

void AresDataCollectorSystem::Log(const char* msg)
{
	if(mv_log.Size()<1000){
		mv_log.Add(new AresDCLogEntry(msg));
	}
}

void AresDataCollectorSystem::PopLog(Vector &v_tmp)
{
	if(mv_log.Size()>0){
		v_tmp.Add(mv_log.Get(0));
		mv_log.Remove(0);
	}
}

AresDCProjectInfoCache* AresDataCollectorSystem::GetProjectCache(void)
{
	return &m_project_info_cache;
}

void AresDataCollectorSystem::AddProcessorJob(ProcessorJob* job)
{
	mv_processor_jobs.Add(job);
}

UINT AresDataCollectorSystem::GetCompletedConnections(void)
{
	int count=0;
	ml_alive_host_connections.StartIteration();
	while(true){
		AresDCConnection *ac=(AresDCConnection*)ml_alive_host_connections.GetCurrent();
		if(!ac)
			break;
		
		if(ac->IsDying()){
			ml_alive_host_connections.RemoveCurrentAndAdvance();  //never check this one again
			continue;
		}
		else{
			if(ac->HaveSharesBeenSent())
				count++;
		}

		ml_alive_host_connections.Advance();
	}

	return count;
}

int AresDataCollectorSystem::GetConnectingSockets(void)
{
	return m_connecting_sockets;
}

InterdictionTargetManager* AresDataCollectorSystem::GetInterdictionTargetManager(void)
{
	return &m_target_manager;
}

bool AresDataCollectorSystem::IsController(void)
{
	return mb_controller;
}

SupernodeServerSystem* AresDataCollectorSystem::GetSupernodeServerSystem(void)
{
	return &m_supernode_server_system;
}

SupernodeServerSystemInterface* AresDataCollectorSystem::GetSupernodeServerSystemInterface(void)
{
	return &m_supernode_server_system_interface;
}
