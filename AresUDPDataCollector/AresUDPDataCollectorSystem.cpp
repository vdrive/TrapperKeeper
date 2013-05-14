#include "StdAfx.h"
#include "aresudpdatacollectorsystem.h"

//static variable initialization
UINT AresUDPDataCollectorSystem::sm_system_count=0;
AresUDPDataCollectorSystem* AresUDPDataCollectorSystem::sm_system=NULL;
CCriticalSection AresUDPDataCollectorSystem::sm_system_lock;

HANDLE sm_log_file_handle=NULL;
UINT sm_log_bytes_written=0;
CCriticalSection sm_log_lock;

AresUDPDataCollectorSystem::AresUDPDataCollectorSystem(void)
{
	mb_processor_ready=false;
}

AresUDPDataCollectorSystem::~AresUDPDataCollectorSystem(void)
{
}

UINT AresUDPDataCollectorSystem::Run(void)
{
	if(!this->b_killThread){
		LogConnectionInfo("Loading Ares virtual machine...");
		mp_processor=new Processor();
		LogConnectionInfo("Finished loading Ares virtual machine.");
		mb_processor_ready=true;
	}	

	if(this->b_killThread)
		return 0;

	//start the udp system, since the processor is now ready to serve it
	m_searcher.StartSystem();

	Timer last_update;

	while(!this->b_killThread){
		Sleep(25);
		m_searcher.Update();
		
		if(last_update.HasTimedOut(1)){
			last_update.Refresh();
			Update();
		}
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

void AresUDPDataCollectorSystem::LogConnectionInfo(const char* str)
{
	//TRACE("Log connection info: %s\n",str);
	static connection_entry_num=0;

	TRACE("%s\n",str);
//	if(mv_connection_log.Size()<50)
//		mv_connection_log.Add(new LogEntry(str,connection_entry_num++));
}

void AresUDPDataCollectorSystem::AddProcessorJob(ProcessorJob* job)
{
	mv_processor_jobs.Add(job);
}

void AresUDPDataCollectorSystem::StartSystem(void)
{
	this->StartThreadLowPriority();
	m_udp_dispatcher.StartSystem();
	m_search_manager.StartThreadLowPriority();
	m_host_cache.StartSystem();
	m_search_processor.StartThreadLowPriority();
}

void AresUDPDataCollectorSystem::StopSystem(void)
{
	//signal certain threads to start shutting down
	this->b_killThread=1;
	m_search_processor.b_killThread=1;
	m_search_manager.b_killThread=1;

	//make sure all threads are dead
	m_udp_dispatcher.StopSystem();
	m_search_manager.StopThread();
	m_host_cache.StopSystem();
	m_search_processor.StopThread();
	this->StopThread();
}

UDPDispatcher* AresUDPDataCollectorSystem::GetUDPDispatcher(void)
{
	return &m_udp_dispatcher;
}

void AresUDPDataCollectorSystem::Log(const char* str)
{
	TRACE("%s\n",str);
}

SearchManager* AresUDPDataCollectorSystem::GetSearchManager(void)
{
	return &m_search_manager;
}

void AresUDPDataCollectorSystem::LogToFile(const char* str)
{
	TRACE("%s\n",str);
}

Buffer2000* AresUDPDataCollectorSystem::GetGUIDBuffer(void)
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

AresUDPDCHostCache* AresUDPDataCollectorSystem::GetHostCache(void)
{
	return &m_host_cache;
}

void AresUDPDataCollectorSystem::Update(void)
{
	if(!mb_processor_ready || !m_host_cache.IsReady()){
		return;  //nothing to update and we don't want to make any new connections yet
	}



	//TYDEBUG
	static bool b_reached_quota=false;
//	if(m_udp_system.GetAliveHostCount()>=300)
//		b_reached_quota=true;

	bool b_using_tcp=true;  //toggle for testing with and without TCP/IP

	UINT udp_connection_limit=2000;

	for(int i=0;i<15 && !b_reached_quota && mv_processor_jobs.Size()<15 && (m_searcher.GetAliveHostCount()<udp_connection_limit);i++){  //open X new connections per update

		Vector v_tmp;
		m_host_cache.GetNextHost(v_tmp,false);

		if(v_tmp.Size()>0){
			
			//room to add more udp contacts
			AresHost* host=(AresHost*)v_tmp.Get(0);

			m_searcher.AddHost(host->GetIP(),host->GetPort(),false);
		}
	}

}

SearchResultProcessor* AresUDPDataCollectorSystem::GetSearchProcessor(void)
{
	return &m_search_processor;
}
