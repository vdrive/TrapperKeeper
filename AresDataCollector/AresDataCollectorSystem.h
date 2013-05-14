#pragma once
#include "..\tkcom\ThreadedObject.h"
#include <afxmt.h>
#include "AresDCHostCache.h"
#include "AresDCProjectInfoCache.h"
#include "..\tkcom\ServerSocket.h"
#include "..\tkcom\LinkedList.h"
#include "..\tkcom\Timer.h"
#include "Processor.h"
#include "ProcessorJob.h"
#include "InterdictionTargetManager.h"
#include "AresDCInterdictionServer.h"

#include "SupernodeServerSystem.h"
#include "SupernodeServerSystemInterface.h"


#define MAXSYSTEMIPS 32
#include "AresDCLogEntry.h"

class AresDataCollectorSystem : public ThreadedObject, public SocketEventListener
{
private:


	SupernodeServerSystem m_supernode_server_system;
	SupernodeServerSystemInterface m_supernode_server_system_interface;
	bool mb_controller;

	AresDCHostCache m_host_cache;

	AresDCProjectInfoCache m_project_info_cache;
	
	ServerSocket m_server_socket;
	LinkedList ml_host_connections;
	LinkedList ml_alive_host_connections;
	int m_connecting_sockets;
	Vector mv_connections;
	Timer m_connection_purge_timer;
	bool mb_force_connection_purge;
	string m_status;
	bool IsConnectedToUltraPeer(const char* host_ip);


	Timer m_last_search;
	vector<string> mv_system_ips;
	Vector mv_log;

	Vector mv_processor_jobs;
	

	Processor *mp_processor;

	bool mb_processor_ready;

	InterdictionTargetManager m_target_manager;
	AresDCInterdictionServer m_interdiction_server;

public:
	void OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection);

	AresDataCollectorSystem(void);
	~AresDataCollectorSystem(void);
	UINT Run(void);
	void StartSystem(void);
	void StopSystem(void);

//************ System Singleton Paradigm *****************//
	static CCriticalSection sm_system_lock;
	static void ReleaseSystem(){
		ASSERT(sm_system_count>0);
		//CSingleLock lock(&sm_system_lock,TRUE);
		sm_system_count--;
		if(sm_system_count==0){
			
			TRACE("AresDataCollectorSystem:  AresDataCollectorSystem Static Object Destroyed.\n");
			delete sm_system;
			sm_system=NULL;
		}
		
		if(sm_system!=NULL){
			//CString log_msg;
			//log_msg.Format("SpoofingSystem::ReleaseSystem() %d references.",sm_system_count);
			//sm_system->LogToFile(log_msg);
		}
	}

	static AresDataCollectorSystem* GetSystem(){
		//CSingleLock lock(&sm_system_lock,TRUE);
		if(sm_system==NULL){
			TRACE("AresDataCollectorSystem:  New AresDataCollectorSystem Static Object Created\n");
			sm_system=new AresDataCollectorSystem();
		}
		sm_system_count++;
		//CString log_msg;
		//log_msg.Format("SpoofingSystem::GetSystem() %d references.",sm_system_count);
		//sm_system->LogToFile(log_msg);
		return sm_system;
	}

	static UINT sm_system_count;
	static AresDataCollectorSystem *sm_system;  //this is set to NULL in the .cpp file
//************ End System Singleton Paradigm *************//

	void UpdateConnecting(int dif);
	UINT GetEventCacheTime(void);
	AresDCHostCache* GetHostCache(void);
	const char* GetSystemIP(void);
	void Update(void);
	void Log(const char* msg);
	void PopLog(Vector &v_tmp);
	AresDCProjectInfoCache* GetProjectCache(void);
	void AddProcessorJob(ProcessorJob* job);
	UINT GetCompletedConnections(void);
	int GetConnectingSockets(void);
	InterdictionTargetManager* GetInterdictionTargetManager(void);
	bool IsController(void);
	SupernodeServerSystem* GetSupernodeServerSystem(void);
	SupernodeServerSystemInterface* GetSupernodeServerSystemInterface(void);
};

//accompanying accessor class.  It's extremely simple so it gets to be in with the MetaSystem class which is the sole reason it exists anyways
class AresDataCollectorSystemReference{
private:
	static AresDataCollectorSystem mp_system;

public:
	AresDataCollectorSystemReference(){
		//mp_system=SpoofingSystem::GetSystem();
	}
	~AresDataCollectorSystemReference(){
		//SpoofingSystem::ReleaseSystem();
	}

	//returns a pointer to the one meta system reference
	inline AresDataCollectorSystem* System(){
		return &mp_system;
	}
};
