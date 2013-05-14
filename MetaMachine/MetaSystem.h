#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Vector.h"
#include "DonkeyServer.h"
#include "TinySQL.h"
#include "MetaCom.h"
#include "DataBaseInfo.h"
#include "ProjectCheckSums.h"
#include "ProjectCheckSum.h"
#include "ProjectKeywords.h"
#include "ProjectKeywordsVector.h"
#include "..\tkcom\ThreadedObject.h"

class MetaSystem :	public ThreadedObject
{
private:
	class Query : public Object{
	private:
		friend class MetaSystem;
		string m_query;
	};

private:
	MetaSystem(void);
	~MetaSystem(void);

	Vector mv_servers;
	Vector mv_queued_searches;
	Vector mv_needed_queries;
	bool b_searches_locked;
	Timer m_main_thread_check;

	DataBaseInfo m_data_base_info;
	ProjectChecksums m_project_checksums;
	ProjectKeywordsVector m_project_keywords_vector;
	bool b_update_projects_database;
	//Vector mv_searchable;  //a set of SearchableItems


//************ System Singleton Paradigm *****************//
private:
	friend class MetaSystemReference;  //make tcpreference a friend so it can access these private function.

	static void ReleaseSystem(){
		ASSERT(sm_system_count>0);
		sm_system_count--;
		if(sm_system_count==0){
			TRACE("MetaMachine Service:  MetaSystem Static Object Destroyed.\n");
			delete sm_system;
			sm_system=NULL;
		}
	}

	static MetaSystem* GetSystem(){
		if(sm_system==NULL){
			TRACE("MetaMachine Service:  New MetaSystem Static Object Created\n");
			sm_system=new MetaSystem();
		}
		sm_system_count++;
		return sm_system;
	}

	static UINT sm_system_count;
	static MetaSystem *sm_system;  //this is set to NULL in the .cpp file
//************ End System Singleton Paradigm *************//


public:
	void AddServer(const char* ip, unsigned short port);
	void NewConnection(UINT handle, const char* source_ip);
	void NewData(UINT handle, const char* source_ip, Buffer2000& data);
	void LostConnection(UINT handle, const char* source_ip);
	
	void Update(void);
	//void NewSupply(UINT job_reference,SearchResult *result);
	//void NewSupplySource(byte *hash, UINT ip);
	void SaveToServer(void);

private:
	void SearchedProject(const char* project);
	UINT Run();
	//Vector mv_search_tasks;
	bool mb_init;
	Timer m_last_rawsupply_table_clean;

	CTime m_last_save;
	
	void Init(void);
public:
	void CleanUp(void);
	void CreateSearches(void);
	void ComDataReceived(MetaCom *com, char* source_ip, byte* data, UINT data_length);
public:
	void UpdateProjectInfoTree(CTreeCtrl& tree);
private:
	void InitSearch(void);
	void SaveChecksumsToDisk(void);
	void SaveKeywordsToDisk(void);
public:
	void LoadControllerInfo(void);
	void AddQuery(const char* query);
	void ReportAllResults(void);
	void UpdateServerTree(CTreeCtrl& tree);
	void BuildSpoofTable(void);
	CString GenerateRandomHash(void);
	bool IsMainThreadDead(void);
	void UpdateProjectsDatabase(void);
	void BuildSwarmTable(void);
	void BuildFalseDecoyTable(void);
};

//accompanying accessor class.  It's extremely simple so it gets to be in with the MetaSystem class which is the sole reason it exists anyways
class MetaSystemReference{
private:
	MetaSystem* mp_system;

public:
	MetaSystemReference(){
		mp_system=MetaSystem::GetSystem();
	}
	~MetaSystemReference(){
		MetaSystem::ReleaseSystem();
	}

	//returns a pointer to the one meta system reference
	inline MetaSystem* System(){
		return mp_system;
	}
};

