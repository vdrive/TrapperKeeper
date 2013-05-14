#pragma once
#include "..\tkcom\ThreadedObject.h"
#include "..\tkcom\Vector.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\TKComInterface.h"
#include "Source.h"

//current strategy:
// use source class, let it scan the distribute directory.  remove the extra files we don't want to take into consideration.
// seperate the different sub directories into

class LoadBalanceSystem : public ThreadedObject
{
	class MapHolder : public Object{
	public:
		TKSyncherMap m_media_map;
		TKSyncherMap m_swarmer_map;
	};
	class ProjectInfo : public Object{
	public:
		string m_project_name;
		int m_decoy_load;
		int m_swarm_load;
	};

	class FileHost : public Object{
	private:
		bool mb_pinged;
		CTime m_last_ping;
		CTime m_last_ping_response;
	public:
		string m_ip;
		MapHolder *mp_map_holder;
		//TKSyncherMap m_media_map;   // a map for the media maker
		//TKSyncherMap m_swarmer_map; // a map for the swarmer
		

		inline bool IsDead(){
			if(mb_pinged!=false && m_last_ping_response<m_last_ping){  //if no response for 3 hours, assume its dead
				return true;
			}
			else return false;
		}
		inline void SentPing(){
			m_last_ping=CTime::GetCurrentTime();
			mb_pinged=true;
		}

		inline void GotPingResponse(){
			m_last_ping_response=CTime::GetCurrentTime();
		}

		FileHost(const char *ip)
		: m_last_ping(1980,1,1,1,1,1)
		, m_last_ping_response(1980,2,2,2,2,2)
		{
			mp_map_holder=NULL;
			m_ip=ip;
			mb_pinged=false;
		}

		inline int CompareTo(Object *object){
			FileHost *host=(FileHost*)object;
			return stricmp(m_ip.c_str(),host->m_ip.c_str());
		}

		bool HasProject(const char* query_project)
		{
			if(mp_map_holder->m_media_map.HasDirectory(query_project) || mp_map_holder->m_swarmer_map.HasDirectory(query_project))
				return true;
			else return false;
		}
	};
private:
	Vector mv_map_holders;
	Vector mv_hosts;
	Vector mv_project_info;
	vector <string> mv_unassigned_projects;
	UINT m_file_limit;
	Source m_main_source;
	bool mb_hosts_assigned;
	UINT m_desired_average_rack_load;
	UINT m_actual_average_rack_load;

	//void RenameFiles(TKSyncherMap& map);
	LoadBalanceSystem::MapHolder* GenerateMapHolder();
public:
	void Update(TKComInterface *p_com);
	UINT Run();
	LoadBalanceSystem(void);
	~LoadBalanceSystem(void);
	void Shutdown(void);
	void Startup(void);
	byte* GetHostMap(char* source_ip,const char* source_name,int& buffer_length);
	
private:
	int RemoveRandomProject(MapHolder* host,bool b_swarm);
	LoadBalanceSystem::ProjectInfo* GetProjectInfo(const char* project_name);
public:
	void GotPingResponse(const char* source_ip);
	void FillProjectInfoTree(CTreeCtrl* p_tree);
	void FillRackInfoTree(CTreeCtrl* p_tree);
	UINT GetActualRackLoad(void);
	UINT GetDesiredRackLoad(void);
	
	void GetUnassignedProjects(vector<string> &v_projects);
private:
	void AddUnassignedProject(const char* project_name);
};
