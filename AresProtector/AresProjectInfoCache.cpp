#include "StdAfx.h"
#include ".\AresProjectInfoCache.h"
#include "..\tkcom\TinySQL.h"
#include "mmsystem.h"
#include "AresProjectInfo.h"
#include "aresprojectinfocache.h"

AresProjectInfoCache::AresProjectInfoCache(void)
{
	mb_ready=false;
}

AresProjectInfoCache::~AresProjectInfoCache(void)
{
}

UINT AresProjectInfoCache::Run(){
	srand(timeGetTime());

	int init_delay=100+rand()%LOADDELAY;  //TYDEBUG
	Timer startup;

	while(!startup.HasTimedOut(init_delay) & !this->b_killThread){
		Sleep(1000);
	}

	Timer last_update;
	bool b_has_updated=false;

	while(!this->b_killThread){
		if(last_update.HasTimedOut(60*60*18) || !b_has_updated || (mv_projects.Size()==0 && last_update.HasTimedOut(60*60*2))){  //TYDEBUG
			last_update.Refresh();
			b_has_updated=true;

			//TYDEBUG
			/*
			TinySQL sql;
			sql.Init("38.119.64.99","onsystems","ebertsux37","project_management",3306);

			if(!sql.Query("select project_name,owner,artist,id from projects where active='A'",true))
				continue;  //don't clear our old projects if we can't get any new ones right now

			Vector v_tmp_projects;
			
			for(int i=0;i<(int)sql.m_num_rows;i++){
				CString project=sql.mpp_results[i][0].c_str();
				CString artist=sql.mpp_results[i][2].c_str();
				CString owner=sql.mpp_results[i][1].c_str();
				int id=atoi(sql.mpp_results[i][3].c_str());
				
				//mv_projects
				v_tmp_projects.Add(new AresProjectInfo(project,owner,artist,id));
			}


			if(!sql.Query("select project_id,name_id from action_items where name_id=55 OR name_id=54",true))
				continue;

			vector<UINT> v_tmp_ids;
			vector<bool> vb_swarm;
			vector<bool> vb_decoy;
			for(int i=0;i<(int)sql.m_num_rows;i++){
				UINT id=atoi(sql.mpp_results[i][0].c_str());
				string tag=sql.mpp_results[i][1].c_str();
				if(stricmp(tag.c_str(),"55")==0){
					vb_swarm.push_back(true);
					vb_decoy.push_back(false);
				}
				else{
					vb_decoy.push_back(true);
					vb_swarm.push_back(false);
				}

				v_tmp_ids.push_back(id);
			}

			//check to see if each project actually has ares protection turned on
			for(int j=0;j<(int)v_tmp_projects.Size();j++){
				AresProjectInfo *proj_info=(AresProjectInfo*)v_tmp_projects.Get(j);
				bool b_found=false;
				for(int i=0;i<(int)v_tmp_ids.size();i++){
					if(proj_info->m_id==v_tmp_ids[i]){
						//if(vb_swarm[i])
						//	proj_info->mb_swarming=true;
						//if(vb_decoy[i])
						//	proj_info->mb_decoying=true;
						b_found=true;
					}
				}
				if(!b_found){
					v_tmp_projects.Remove(j);  //not doing ares protection
					j--;
				}
			}

			CSingleLock lock(&m_lock,TRUE);
			mv_projects.Clear();
			mv_projects.Copy(&v_tmp_projects);

			TRACE("AresProjectInfoCache::Run() Loaded %d projects\n",v_tmp_projects.Size());
			*/
			mb_ready=true;
		}
		Sleep(100);
	}
	
	return 0;
}

void AresProjectInfoCache::GetProjects(Vector& v)
{
	CSingleLock lock(&m_lock,TRUE);
	v.Copy(&mv_projects);
}

bool AresProjectInfoCache::IsReady(void)
{
	//TYDEBUG
//	if(mv_projects.Size()<1)
//		return false;
//	else
	return mb_ready;
}

string AresProjectInfoCache::GetArtist(const char* project)
{
	CSingleLock lock(&m_lock,TRUE);
	for(int i=0;i<(int)mv_projects.Size();i++){
		AresProjectInfo* pi=(AresProjectInfo*)mv_projects.Get(i);
		if(stricmp(pi->m_name.c_str(),project)==0)
			return pi->m_artist;
	}
	return string();
}

bool AresProjectInfoCache::IsProjectActive(const char* project)
{
	//ROBOTS-2005-03-03
	CSingleLock lock(&m_lock,TRUE);
	for(int i=0;i<(int)mv_projects.Size();i++){
		AresProjectInfo* pi=(AresProjectInfo*)mv_projects.Get(i);
		if(stricmp(pi->m_name.c_str(),project)==0)
			return true;
	}
	return false;
}
