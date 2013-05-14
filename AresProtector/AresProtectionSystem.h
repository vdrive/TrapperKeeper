#pragma once
#include <afxmt.h>
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\linkedlist.h"
#include "AresHostCache.h"
#include "AresConnection.h"
#include "AresFTConnection.h"
#include "AresInterdictionConnection.h"
#include "AresSpoofCache.h"
#include "AresProjectInfoCache.h"
#include "..\AresDataCollector\ProcessorJob.h"
#include "..\AresDataCollector\Processor.h"
#include "AresInterdictionManager.h"
#include "AresTCP.h"
#include "AresDecoyPool.h"

#define MAXSYSTEMIPS 64
#define NUMCONBINS 1000

class AresProtectionSystem :  public ThreadedObject
{
	class HashCacheObject : public Object{
	public:
		byte m_hash[20];
		int m_length;
		int m_start;
		HashCacheObject(byte* hash,int length,int start){
			m_length=length;
			memcpy(m_hash,hash,20);
			m_start=start;
		}
	};

	class InterdictionQueueObject : public Object{
	public:
		Vector m_target;
		CTime m_time;
		InterdictionQueueObject(InterdictionTarget* target,int poll){
			m_time=CTime::GetCurrentTime()+CTimeSpan(0,0,0,poll);
			m_target.Add(target);
		}
	};

private:

	AresDecoyPool m_decoy_pool;

	Vector mv_ban_clients[NUMCONBINS];
	Vector mv_interdiction_connections[NUMCONBINS];
	Vector mv_host_connections[NUMCONBINS];
	Vector mv_alive_host_connections[NUMCONBINS];
	Vector mv_ft_connections[NUMCONBINS];
	Vector mv_interdiction_queue[NUMCONBINS];

	UINT m_num_host_connections;
	UINT m_num_alive_host_connections;
	UINT m_num_ft_connections;
	UINT m_num_interdiction_connections;
	UINT m_num_interdiction_queued;


	AresTCP m_tcp_system;
	
	LinkedList ml_cache_hashes[256];

	vector<string> mv_system_ips;

	string m_user_names[30];
	//string m_user_name2;

	AresInterdictionManager m_interdiction_manager;

	UINT m_interdiction_transfer_amount;

	bool mb_processor_ready;

	//byte m_data_256K_hash[20];
	//byte m_data_1Meg_hash[20];
	//byte m_data_2Meg_hash[20];
	//byte m_data_5Meg_hash[20];

	Buffer2000 m_mp3_noise;

	AresHostCache m_host_cache;
	AresSpoofCache m_ares_spoof_cache;
	AresProjectInfoCache m_ares_project_info_cache;

	int m_event_cache_time;
	//LinkedList ml_host_connections;
	//LinkedList ml_interdiction_connections;
	//LinkedList ml_alive_host_connections;
	//LinkedList ml_ft_connections;

	int m_connecting_sockets;
	//LinkedList ml_servers;
	vector <UINT> mv_servers;
	

	LinkedList ml_data_sent_log;
	UINT m_total_hash_cache_hits;
	UINT m_found_hash_cache_hits;
	
	string m_status;
	Timer m_connection_purge_timer;
	bool mb_force_connection_purge;

	bool mb_has_project_info;

	Processor *mp_processor;

	Vector mv_processor_jobs;
	

public:
	inline const char* GetStatus(){return m_status.c_str();}

public:
	UINT Update();  //previously a run

public:
	AresProtectionSystem(void);
	~AresProtectionSystem(void);

	UINT Run();

	void OnAccept(UINT server_handle,UINT con_id,const char* peer,unsigned short port);

//	CCriticalSection m_tcp_lock;
//************ System Singleton Paradigm *****************//
	
private:

	friend class AresProtectionSystemReference;
	static CCriticalSection sm_system_lock;

	static void ReleaseSystem(){
		ASSERT(sm_system_count>0);
		//CSingleLock lock(&sm_system_lock,TRUE);
		sm_system_count--;
		if(sm_system_count==0){
			
			TRACE("AresProtectionService:  AresProtectionSystem Static Object Destroyed.\n");
			delete sm_system;
			sm_system=NULL;
		}
		
		if(sm_system!=NULL){
			//CString log_msg;
			//log_msg.Format("SpoofingSystem::ReleaseSystem() %d references.",sm_system_count);
			//sm_system->LogToFile(log_msg);
		}
	}
	


	
	static AresProtectionSystem* GetSystem(){
		//CSingleLock lock(&sm_system_lock,TRUE);
		if(sm_system==NULL){
			TRACE("AresProtectionService:  New AresProtectionSystem Static Object Created\n");
			sm_system=new AresProtectionSystem();
		}
		sm_system_count++;
		//CString log_msg;
		//log_msg.Format("SpoofingSystem::GetSystem() %d references.",sm_system_count);
		//sm_system->LogToFile(log_msg);
		return sm_system;
	}


	static UINT sm_system_count;
	static AresProtectionSystem *sm_system;  //this is set to NULL in the .cpp file
	
public:
//************ End System Singleton Paradigm *************//

	void StartSystem(void);
	void StopSystem(void);
//	AresTCP* GetTCP(void);
	void SetEventCacheTime(int val);
	int GetEventCacheTime(void);
	AresDecoyPool* GetDecoyPool();
	Vector* GetConnectedHosts(void);
	AresConnection* GetHost(UINT id);
	AresHostCache* GetHostCache(void);
	void ResetConnectionPurgeTimer(void);
	void AddProcessorJob(ProcessorJob* job);
private:
	bool IsConnectedToUltraPeer(const char* host_ip);
public:
	void ForceConnectionPurge(void);
	void GetSomeSpoofs(Vector& v_file_group);
	AresProjectInfoCache* GetProjectInfoCache(void);
	void UpdateConnecting(int dif);
	unsigned int GetSpoofsLoaded(void);
	Vector* GetFTConnections(void);
	UINT GetFileLength(byte* hash);
	void InitMP3Noise(void);
	Buffer2000* GetMP3NoiseBuffer(void);
	const char* GetSystemIP(void);
	void CleanDirectory(const char* directory);
	string GetUserName(UINT my_listening_port);
	//string GetUserName2(void);
	//void InitUserName(void);
	//byte* Get1MegHash(void);
	//byte* Get2MegHash(void);
	//byte* Get5MegHash(void);
	void AddDataSentEvent(UINT amount);
	UINT GetFTDataPerSecond(void);
	void UpdateFileTransferConnections(void);
	void CalculateHash( int start,int data_length,byte *hash);
	bool IsIPOkay(const char* ip);
	void AddInterdictionTransferAmount(UINT amount);
	UINT GetInterdictionTransferAmount(void);
	void UpdateInterdictionConnections(void);
	void AddInterdictionConnection(InterdictionTarget* target);
	UINT GetInterdictionConnectionCount(void);
	UINT GetInterdictionConnectionTransferredCount(void);
	float GetBannedClientRatio(void);
	UINT GetInterdictionTargetCount(void);
	bool IsInterdicting(const char* str_ip,unsigned short port);
	void AddInterdictionQueueTarget(InterdictionTarget* it, int poll);
	UINT GetInterdictionQueueSize(void);
	AresTCP* GetTCPSystem(void);
	int GetBinFromPeer(const char* peer);
	void OnReceive(UINT con_id , byte* data , UINT length,const char* peer);
	void OnClose(UINT con_id,UINT reason, const char* peer);
	void OnConnect(UINT con_id, const char* peer);
	void UpdateTCP(void);
	bool IsClientBanned(const char* ip, byte* hash);
	void BanClient(const char* ip, byte* hash);
	float GetHashCacheEfficiency(void);
};

//accompanying accessor class.  It's extremely simple so it gets to be in with the AresSystem class which is the sole reason it exists anyways
class AresProtectionSystemReference{
public:
	AresProtectionSystem *mp_system;

	
public:
	AresProtectionSystemReference(){
		mp_system=AresProtectionSystem::GetSystem();
	}
	~AresProtectionSystemReference(){
		AresProtectionSystem::ReleaseSystem();
	}

	//returns a pointer to the one meta system reference
	inline AresProtectionSystem* System(){
		return mp_system;
	}
};

