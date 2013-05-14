#pragma once

#include "..\tkcom\vector.h"
#include "..\tkcom\ThreadedObject.h"
#include "..\tkcom\buffer2000.h"
#include <afxmt.h>
#include <mmsystem.h>

class HashSetCollectionSystem : public ThreadedObject
{
public:
	HashSetCollectionSystem(void);
	~HashSetCollectionSystem(void);

	void NewConnection(UINT handle, const char* source_ip);
	void NewData(UINT handle, const char* source_ip, Buffer2000& data);
	void LostConnection(UINT handle, const char* source_ip);
	

//************ System Singleton Paradigm *****************//
protected:
	UINT Run();
private:
	void LoadDecoys(const char* in_directory,Vector &v_decoy_vector);
	//void LoadSwarms(const char* in_directory,Vector &v_swarm_vector);
	CCriticalSection m_decoys_lock;
	bool mb_needs_decoy_load;
	Vector mv_servers;  //a vector of donkey servers
	//vector <UINT> mv_rogue;
	friend class HashSetCollectionSystemReference;  //make tcpreference a friend so it can access these private function.
	Vector mv_decoys;  //a vector of donkey files representing decoys
	Vector mv_spoofs;  
	Vector mv_swarms;  //a vector of donkey files representing swarms
	Vector mv_client_connections;  //a vector of client connections

	static void ReleaseSystem(){
		ASSERT(sm_system_count>0);
		sm_system_count--;
		if(sm_system_count==0){
			TRACE("MetaMachine HashSet Collection Service:  HashSetCollectionSystem Static Object Destroyed.\n");
			delete sm_system;
			sm_system=NULL;
		}
	}

	static HashSetCollectionSystem* GetSystem(){
		if(sm_system==NULL){
			TRACE("MetaMachine HashSet Collection Service:  New HashSetCollectionSystem Static Object Created\n");
			sm_system=new HashSetCollectionSystem();
		}
		sm_system_count++;
		return sm_system;
	}

	static UINT sm_system_count;
	static HashSetCollectionSystem *sm_system;  //this is set to NULL in the .cpp file
//************ End System Singleton Paradigm *************//

public:
	void Init(void);
	void Update(void);
	void GetConnectedServers(vector <string>& v_servers);
	void GetDisconnectedServers(vector <string>& v_servers);
	void AddServer(const char* ip, UINT port);
	void Shutdown(void);
	void AddHash(byte hash, string name, int size);
	
	void GetSomeDecoys(int chance, int limit,Vector& v);
protected:
	void InitServers(void);
public:
};

//accompanying accessor class.  It's extremely simple so it gets to be in with the MetaSystem class which is the sole reason it exists anyways
class HashSetCollectionSystemReference{
private:
	HashSetCollectionSystem* mp_system;

public:
	HashSetCollectionSystemReference(){
		mp_system=HashSetCollectionSystem::GetSystem();
	}
	~HashSetCollectionSystemReference(){
		HashSetCollectionSystem::ReleaseSystem();
	}

	//returns a pointer to the one meta system reference
	inline HashSetCollectionSystem* System(){
		return mp_system;
	}
};

