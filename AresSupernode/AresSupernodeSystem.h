#pragma once
#include <afxmt.h>
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\SocketEventListener.h"
#include "AresSNSpoofCache.h"
#include "AresSNHostCache.h"
#include "SupernodeUDPSystem.h"
#include "AresSupernodeUDPDispatcher.h"
#include "AresDecoyPoolCache.h"



#ifdef TKSOCKETSYSTEM
#include "AresSNTCP.h"
#endif


#include "..\AresDataCollector\ProcessorJob.h"
#include "..\AresDataCollector\Processor.h"
#include "AresSNLogEntry.h"

class AresSupernodeSystem : ThreadedObject, public SocketEventListener
{
private:
	AresDecoyPoolCache m_decoy_cache;
	AresSupernodeSystem(void);
	~AresSupernodeSystem(void);
	AresSNSpoofCache m_spoof_cache;
	AresSNHostCache m_host_cache;
	Processor *mp_processor;
	SupernodeUDPSystem m_udp_system;
	AresSupernodeUDPDispatcher m_udp_dispatcher;
#ifdef TKSOCKETSYSTEM
	AresSNTCP m_tcp_system;
#endif
	vector<string> mv_system_ips;
	string m_user_name;
	Vector mv_processor_jobs;
	Vector mv_query_log;
	Vector mv_connection_log;
	Vector mv_server_sockets;

	UINT m_forward_index1,m_forward_index2;
	

	Buffer2000 m_guid_buffer;
	bool mb_processor_ready;

	Vector mv_sn_connections[NUMCONBINS];
	Vector mv_client_connections[NUMCONBINS];
	UINT m_num_host_connections;

	bool IsConnectedToUltraPeer(const char* host_ip);

public:
	inline int GetBinFromPeer(const char* peer)
	{
		UINT ip=inet_addr(peer);
		return (int)(ip%NUMCONBINS);
	}

	void OnReceive(UINT con_id , byte* data , UINT length,const char* peer);
	void OnClose(UINT con_id,UINT reason, const char* peer);
	void OnConnect(UINT con_id, const char* peer);

	void OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection);

	void AddProcessorJob(ProcessorJob* job);

//************ System Singleton Paradigm *****************//
	static CCriticalSection sm_system_lock;
	static void ReleaseSystem(){
		CSingleLock lock(&sm_system_lock,TRUE);
		sm_system_count--;
		if(sm_system_count==0){
			
			TRACE("Ares Supernode Service:  AresSupernodeSystem Static Object Destroyed.\n");
			delete sm_system;
			sm_system=NULL;
		}
	}

	static AresSupernodeSystem* GetSystem(){
		CSingleLock lock(&sm_system_lock,TRUE);
		if(sm_system==NULL){
			TRACE("Ares Supernode Service:  New AresSupernodeSystem Static Object Created\n");
			sm_system=new AresSupernodeSystem();
		}
		sm_system_count++;
		return sm_system;
	}

	static UINT sm_system_count;
	static AresSupernodeSystem *sm_system;  //this is set to NULL in the .cpp file

//************ End System Singleton Paradigm *************//

protected:
	UINT Run(void);

public:
	void StartSystem(void);
	void StopSystem(void);
	
	AresSNSpoofCache* GetSpoofCache(void);


#ifdef TKSOCKETYSTEM
	TKSocketSystem* GetTCPSystem(void);
#endif

	void UpdateTCP(void);
	const char* GetSystemIP(void);
	void InitUserName(void);
	const char* GetUserName(void);
	UINT GetEventCacheTime(void);
	void Update(void);
	UINT GetTotalConnections(void);
	UINT GetFullyConnected(void);
	UINT GetKnownHosts(void);
	void LogQuery(const char* str);
	void LogConnectionInfo(const char* str);
	void PopQueryLog(Vector& v);
	void PopConnectionLog(Vector& v);
	UINT GetLoadedDecoys(void);
	void LogToFile(const char* str);
	UINT GetUDPHostCount(void);
	void SaveHost(const char* host_ip , unsigned short port);
	AresSNHostCache* GetHostCache(void);
	AresSupernodeUDPDispatcher* GetUDPDispatcher(void);
	Buffer2000* GetGUIDBuffer(void);
	void ForwardTCPQuery(Buffer2000* query);
	AresDecoyPoolCache* GetDecoyCache(void);
};

//accompanying accessor class.  It's extremely simple so it gets to be in with the MetaSystem class which is the sole reason it exists anyways
class AresSupernodeSystemRef{
private:
	AresSupernodeSystem* mp_system;

public:

	AresSupernodeSystemRef(){
		mp_system=AresSupernodeSystem::GetSystem();
	}

	~AresSupernodeSystemRef(){
		AresSupernodeSystem::ReleaseSystem();
	}

	//returns a pointer to the one meta system reference
	inline AresSupernodeSystem* System(){
		return mp_system;
	}
};