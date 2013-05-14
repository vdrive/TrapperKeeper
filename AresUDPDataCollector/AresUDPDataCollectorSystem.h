#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\AresDataCollector\Processor.h"
#include "UDPDispatcher.h"
#include "SearchManager.h"
#include "AresUDPDCHostCache.h"
#include "UDPSearcher.h"
#include "SearchResultProcessor.h"
#include "..\tkcom\buffer2000.h"

#include "..\AresDataCollector\ProcessorJob.h"

class AresUDPDataCollectorSystem : public ThreadedObject
{
	class LogEntry : public Object{
	public:
		string m_log;
		LogEntry(const char* str,UINT entry_num){
			CString output;
			CTime cur_time=CTime::GetCurrentTime();
			CString cur_time_str=cur_time.Format("%A %I:%M %p");
			if(entry_num>0)
				output.Format("[%u] %s:  %s\r\n",entry_num,cur_time_str,str);
			else
				output.Format("%s:  %s\r\n",cur_time_str,str);
			m_log=(LPCSTR)output;
		}
	};
protected:
	Vector mv_processor_jobs;
	bool mb_processor_ready;
	Processor *mp_processor;
	UDPDispatcher m_udp_dispatcher;
	SearchManager m_search_manager;
	SearchResultProcessor m_search_processor;
	UDPSearcher m_searcher;
	AresUDPDCHostCache m_host_cache;
	Buffer2000 m_guid_buffer;

public:
	AresUDPDataCollectorSystem(void);
	~AresUDPDataCollectorSystem(void);
	UINT Run(void);
	void AddProcessorJob(ProcessorJob* job);
	void LogConnectionInfo(const char* str);	
	void StartSystem(void);
	void StopSystem(void);
	UDPDispatcher* GetUDPDispatcher(void);
	Buffer2000* GetGUIDBuffer(void);

//************ System Singleton Paradigm *****************//
	static CCriticalSection sm_system_lock;
	static void ReleaseSystem(){
		CSingleLock lock(&sm_system_lock,TRUE);
		sm_system_count--;
		if(sm_system_count==0){
			
			TRACE("Ares UDP DataCollector Service:  AresUDPDataCollectorSystem Static Object Destroyed.\n");
			delete sm_system;
			sm_system=NULL;
		}
	}

	static AresUDPDataCollectorSystem* GetSystem(){
		CSingleLock lock(&sm_system_lock,TRUE);
		if(sm_system==NULL){
			TRACE("Ares UDP DataCollector Service:  New AresUDPDataCollectorSystem Static Object Created\n");
			sm_system=new AresUDPDataCollectorSystem();
		}
		sm_system_count++;
		return sm_system;
	}

	static UINT sm_system_count;
	static AresUDPDataCollectorSystem *sm_system;  //this is set to NULL in the .cpp file

//************ End System Singleton Paradigm *************//

	void Log(const char* str);
	SearchManager* GetSearchManager(void);
	void LogToFile(const char* str);
	AresUDPDCHostCache* GetHostCache(void);
	void Update(void);
	SearchResultProcessor* GetSearchProcessor(void);
};

//accompanying accessor class.  It's extremely simple so it gets to be in with the MetaSystem class which is the sole reason it exists anyways
class SystemRef{
private:
	AresUDPDataCollectorSystem* mp_system;

public:

	SystemRef(){
		mp_system=AresUDPDataCollectorSystem::GetSystem();
	}

	~SystemRef(){
		AresUDPDataCollectorSystem::ReleaseSystem();
	}

	//returns a pointer to the one meta system reference
	inline AresUDPDataCollectorSystem* System(){
		return mp_system;
	}
};