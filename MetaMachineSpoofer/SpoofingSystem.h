#pragma once

#include "..\tkcom\linkedlist.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\ThreadedObject.h"
#include "..\tkcom\buffer2000.h"
#include "FileRequestLogger.h"
#include <afxmt.h>
#include <mmsystem.h>
#include "SpoofData.h"
#include "SwarmData.h"
#include "FalseDecoyData.h"
#include "MetaSpooferCom.h"
//#include "TKTCP.h"
#include "..\sampleplugin\dll.h"
#include "ServerSocketBundle.h"
#include "..\tkcom\PeerSocket.h"
#include "SocketEventListener.h"
#include "SearchResult.h"
#include "SearchTask.h"
#include "DonkeyFile.h"
#include "AntiLeakData.h"

#define ED2KGETSWARMS					1
#define ED2KADDSOURCE					2
#define ED2KREMOVESOURCE				3
#define ED2KGETINTERDICTIONTARGETS		4
#define ED2KGETSOURCEHASHES				5
#define ED2KGETFALSEDECOYS				6
#define ED2KGETSPOOFS					7
#define ED2KRACKSERVERLOG				8
#define ED2KSERVERENTRY					9
#define ED2KDECOYLOG					10
#define ED2KSEARCHTASK					11
#define ED2KSEARCHRESULTS				12
#define ED2KGETANTILEAKRARE				13
#define ED2KGETANTILEAKPOPULAR			14
#define ED2KANTILEAKUPLOAD				15
#define ED2KCONTROLKEY					0xe4


class SpoofingSystem : public ThreadedObject, public SocketEventListener
{
	class QueryObject : public Object
	{
	public:
		string m_query;
		QueryObject(const char* query){
			m_query=query;
		}
	};
public:
	void CleanDirectory(const char* directory);

	MetaSpooferCom m_com;

	SpoofingSystem(void);
	~SpoofingSystem(void);

	//bool NewConnection(TKTCPConnection &con,UINT server_handle);
	//void NewData(TKTCPConnection &con, Buffer2000& data);
	//void LostConnection(TKTCPConnection &con);
	byte m_my_hashes[MAXSYSTEMIPS][MULTI_CONNECTION_COUNT][16];  //support for up to MAXSYSTEMIPS system ips
	string m_my_user_names[MAXSYSTEMIPS][MULTI_CONNECTION_COUNT];  //support for up to MAXSYSTEMIPS system ips
	void GetSomeAntiLeakFiles(int chance, int limit,Vector& v);
protected:
	UINT Run();
private:
	bool mb_spoofing_only;
	bool mb_weekend;
	SpoofData m_spoof_data;
	SwarmData m_swarm_data;
	AntiLeakData m_anti_leak_data;
	FalseDecoyData m_false_decoy_data;
	Buffer2000 m_evil_seed_data;
	Buffer2000 m_garbage_data;
	
	ServerSocketBundle m_server_sockets[MULTI_CONNECTION_COUNT];

	vector <string> mv_system_ips;

//	UINT m_queued_data_size;
	//FileRequestLogger m_request_logger;
	
	void LoadDecoys(const char* in_directory,Vector &v_decoy_vector);
	//void LoadSwarms(const char* in_directory,Vector &v_swarm_vector);
	CCriticalSection m_false_decoys_lock;

	CCriticalSection m_weekend_lock;
	//CCriticalSection m_hashes_that_need_sources_lock;
	//CCriticalSection m_interdiction_connection_lock;
	//bool mb_has_decoys_loaded;
	
	//bool mb_needs_decoy_load;
	//bool mb_needs_false_decoy_creation;
	//bool mb_done_initing_servers;
	bool mb_halt_system;
	Vector mv_servers;  //a vector of spoof hosts
	UINT m_emule_version;
	//vector <UINT> mv_rogue;
	friend class MetaSpooferReference;  //make tcpreference a friend so it can access these private function.
	Vector mv_anti_leak_files;  //a vector of donkey files representing decoys
	Vector mv_decoys;  //a vector of donkey files representing decoys
	Vector mv_false_decoys;  //a vector of donkey files representing decoys
	Vector mv_spoofs;  
	Vector mv_swarms;  //a vector of donkey files representing swarms
	LinkedList ml_client_connections;  //a linked list of client connections.  This is way more efficient for periodic insertion and deletion
	Vector mv_interdiction_connections;  //a vector of interdiction connections
	Vector mv_inactive_interdiction_connections;  //a vector of interdiction connections
	Vector mv_interdiction_queue;
	SearchTask m_current_search_task;

	vector <string> mv_hashes_that_need_sources;
	
	//Vector mv_needed_queries;
	

//************ System Singleton Paradigm *****************//
	static CCriticalSection sm_system_lock;
	static void ReleaseSystem(){
		ASSERT(sm_system_count>0);
		//CSingleLock lock(&sm_system_lock,TRUE);
		sm_system_count--;
		if(sm_system_count==0){
			
			TRACE("MetaMachine Spoofer Service:  SpoofingSystem Static Object Destroyed.\n");
			delete sm_system;
			sm_system=NULL;
		}
		
		if(sm_system!=NULL){
			//CString log_msg;
			//log_msg.Format("SpoofingSystem::ReleaseSystem() %d references.",sm_system_count);
			//sm_system->LogToFile(log_msg);
		}
	}

	static SpoofingSystem* GetSystem(){
		//CSingleLock lock(&sm_system_lock,TRUE);
		if(sm_system==NULL){
			TRACE("MetaMachine Spoofer Service:  New SpoofingSystem Static Object Created\n");
			sm_system=new SpoofingSystem();
		}
		sm_system_count++;
		//CString log_msg;
		//log_msg.Format("SpoofingSystem::GetSystem() %d references.",sm_system_count);
		//sm_system->LogToFile(log_msg);
		return sm_system;
	}

	static UINT sm_system_count;
	static SpoofingSystem *sm_system;  //this is set to NULL in the .cpp file
//************ End System Singleton Paradigm *************//

public:
	void Init(Dll *pDLL);
	void Update(void);
	void GetConnectedServers(vector <string>& v_servers);
	void GetDisconnectedServers(vector <string>& v_servers);
	void AddServer(const char* ip, UINT port,bool b_must_add=false,bool b_low_priority=false);
	void Shutdown(void);
	
	void GetSomeDecoys(int chance, int limit,Vector& v);
protected:
	void InitServers(void);
	void OnAccept(CAsyncSocket *cserver_socket,CAsyncSocket* new_connection);
public:
	bool GetSwarmFile(Vector& v, byte* hash);
	bool GetDecoyFile(Vector& v, byte* hash);
	bool GetFalseDecoyFile(Vector& v, byte* hash);

	void AddSwarm(byte* hash, const char* file_name, byte* hash_set, int file_size);
	//FileRequestLogger* GetRequestLogger(void);
	//void AddToQueuedData(UINT amount);
	void GetSomeSpoofs(int chance, int limit,Vector& v);
	void GetSomeSwarms(int chance, int limit,Vector& v);
	//void OnSend(TKTCPConnection &con);
	bool GetUserHash(byte* hash, const char* file_name);
	void WriteUserHash(byte* hash, const char* file_name);
	string GetHashThatNeedsSources(void);
	void AddHashThatNeedsSources(const char* hash);
	void AddSourceForHash(const char* ip, unsigned short int port, const char* hash);
	void RemoveSourceForHash(const char * hash, const char * ip, unsigned short port);
	void AddUserName(const char* name);
	void LogInterdiction(const char* project, const char* hash, const char* ip, unsigned short port, UINT total_downloaded);
	void ComDataReceived(const char* source_ip, byte* data, UINT data_length);
protected:
	void GetInterdictionTargets(void);
	void GetHashesThatNeedSources(void);
	void CreateDecoys(void);
public:
	void LogServerConnection(const char* server_ip);
	void LogNewServer(const char* ip, unsigned short port);
	UINT GetClientConnectionCount(void);
	UINT GetInactiveConnectionCount(void);
	UINT GetActiveConnectionCount(void);
	void SwitchInterdictionConnectionToInactive(Object* con);
	static void LogToFile(const char* str);
	void LogDecoyRequest(const char* file_name,const char* ip);
	UINT GetEmuleVersion(void);
private:
	void InitEvilSeedData(void);
public:
	Buffer2000* GetEvilSeedData(void);
	SearchTask GetSearchTask(void);
	void ReportSearchResults(Vector &v_results);
	void RequestNewSearchTaskFromServer(void);
	//void PostFakeSearchResults(void);
	void AddFalseDecoy(DonkeyFile* df);
	void AddAntiLeakFile(const char* file_name,byte* hash,UINT size);
	void ClearFalseDecoys(void);
	Buffer2000* GetGarbageData(void);
	bool IsWeekend(void);
	bool IsSpoofingOnly(void);
};

//accompanying accessor class.  It's extremely simple so it gets to be in with the MetaSystem class which is the sole reason it exists anyways
class MetaSpooferReference{
private:
	static SpoofingSystem mp_system;

public:
	MetaSpooferReference(){
		//mp_system=SpoofingSystem::GetSystem();
	}
	~MetaSpooferReference(){
		//SpoofingSystem::ReleaseSystem();
	}

	//returns a pointer to the one meta system reference
	inline SpoofingSystem* System(){
		return &mp_system;
	}
};

