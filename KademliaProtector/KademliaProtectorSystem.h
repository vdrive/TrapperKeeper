#pragma once
#include <afxmt.h>
#include "KadUDPListener.h"
#include "KadContactManager.h"
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\SocketEventListener.h"
#include "..\tkcom\ServerSocket.h"
#include "ClientManager.h"
#include "DecoyPool.h"

using namespace Kademlia;

#define MAXSYSTEMIPS 32


class KademliaProtectorSystem : protected ThreadedObject, public SocketEventListener
{
public:
	KademliaProtectorSystem(void);
	~KademliaProtectorSystem(void);

//************ System Singleton Paradigm *****************//
private:
	DecoyPool m_decoy_pool;

	CCriticalSection m_swarm_lock;

	vector <string> mv_system_ips;
	byte m_my_hash[16];

	ServerSocket m_server_sockets[NUMBERPORTS];


	KadUDPListener m_kad_udp_system;
	ClientManager m_client_manager;
	KadContactManager m_kad_contact_manager;

	friend class KademliaProtectorSystemReference;
	static CCriticalSection sm_system_lock;

	static void ReleaseSystem(){
		ASSERT(sm_system_count>0);
		//CSingleLock lock(&sm_system_lock,TRUE);
		sm_system_count--;
		if(sm_system_count==0){
			
			TRACE("KademliaProtectorSystem:  KademliaProtectorSystem Static Object Destroyed.\n");
			delete sm_system;
			sm_system=NULL;
		}
		
		if(sm_system!=NULL){
			//CString log_msg;
			//log_msg.Format("SpoofingSystem::ReleaseSystem() %d references.",sm_system_count);
			//sm_system->LogToFile(log_msg);
		}
	}
	
	static KademliaProtectorSystem* GetSystem(){
		//CSingleLock lock(&sm_system_lock,TRUE);
		if(sm_system==NULL){
			TRACE("KademliaProtectorSystem:  New KademliaProtectorSystem Static Object Created\n");
			sm_system=new KademliaProtectorSystem();
		}
		sm_system_count++;
		//CString log_msg;
		//log_msg.Format("SpoofingSystem::GetSystem() %d references.",sm_system_count);
		//sm_system->LogToFile(log_msg);
		return sm_system;
	}


	static UINT sm_system_count;
	static KademliaProtectorSystem *sm_system;  //this is set to NULL in the .cpp file
//************ End System Singleton Paradigm *************//

public:
	UINT Run(void);
	void StartSystem(void);
	void StopSystem(void);
	KadContactManager* GetContactManager(void);
	const char* GetMyIP(void);
	byte* GetMyHash(void);

	void LogToFile(const char* str);

	bool GetDecoyFile(Vector& v, byte* hash);
	bool GetSwarmFile(Vector& v, byte* hash);

private:
	Vector mv_swarm_files;

private:
	void OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection);
public:
	void ReloadSwarms(void);
	UINT GetFTClientCount(void);
	void GuiClearTerminalFTClients(void);
	UINT GetSwarmCount(void);
	ClientManager* GetClientManager(void);
	KadUDPListener* GetKadUDPSystem(void);
};

//accompanying accessor class.  It's extremely simple so it gets to be in with the AresSystem class which is the sole reason it exists anyways
class KademliaProtectorSystemReference{
public:
	KademliaProtectorSystem *mp_system;

	
public:
	KademliaProtectorSystemReference(){
		mp_system=KademliaProtectorSystem::GetSystem();
	}
	~KademliaProtectorSystemReference(){
		KademliaProtectorSystem::ReleaseSystem();
	}

	//returns a pointer to the one meta system reference
	inline KademliaProtectorSystem* System(){
		return mp_system;
	}
};