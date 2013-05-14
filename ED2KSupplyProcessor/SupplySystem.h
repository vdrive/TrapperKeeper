#pragma once
#include <afxmt.h>
#include "..\tkcom\threadedobject.h"
#include "ProjectManager.h"
#include "ProjectKeywords.h"
#include "ED2KProjectInfo.h"

class SupplySystem : ThreadedObject
{
private:
	SupplySystem(void);
	~SupplySystem(void);

	ProjectManager m_project_manager;
public:

//************ System Singleton Paradigm *****************//
	static CCriticalSection sm_system_lock;
	static void ReleaseSystem(){
		CSingleLock lock(&sm_system_lock,TRUE);
		sm_system_count--;
		if(sm_system_count==0){
			
			TRACE("MetaMachine Spoofer Service:  SpoofingSystem Static Object Destroyed.\n");
			delete sm_system;
			sm_system=NULL;
		}
	}

	static SupplySystem* GetSystem(){
		CSingleLock lock(&sm_system_lock,TRUE);
		if(sm_system==NULL){
			TRACE("MetaMachine Spoofer Service:  New SpoofingSystem Static Object Created\n");
			sm_system=new SupplySystem();
		}
		sm_system_count++;
		return sm_system;
	}

	static UINT sm_system_count;
	static SupplySystem *sm_system;  //this is set to NULL in the .cpp file

//************ End System Singleton Paradigm *************//

protected:
	UINT Run(void);
	void ProcessRawSupply(void);

public:
	void StartSystem(Dll *p_dll);
	void StopSystem(void);
	
	void ProcessSearchResult(ED2KProjectInfo* pi, string& file_name, string& availability, string& hash, string& server_ip, string& time_created, UINT file_size , vector<string> &v_queries,vector<string> &v_cross_name_queries);
private:
	void PrepareStringForDatabase(CString &str);
public:
	string GetProjectTableName(const char* project_name);
};

//accompanying accessor class.  It's extremely simple so it gets to be in with the MetaSystem class which is the sole reason it exists anyways
class SupplySystemRef{
private:
	SupplySystem* mp_system;

public:

	SupplySystemRef(){
		mp_system=SupplySystem::GetSystem();
	}

	~SupplySystemRef(){
		SupplySystem::ReleaseSystem();
	}

	//returns a pointer to the one meta system reference
	inline SupplySystem* System(){
		return mp_system;
	}
};
