#include "StdAfx.h"
#include "searchmanager.h"

#include "..\tkcom\TinySQL.h"
#include "AresUDPDataCollectorSystem.h"
#include "SearchJob.h"
#include "..\AresDatacollector\AresDCProjectInfo.h"

SearchManager::SearchManager(void)
{
	m_num_search_results=0;
}

SearchManager::~SearchManager(void)
{
}

UINT SearchManager::Run(void)
{
	Timer reload_timer;
	bool b_has_loaded=false;


	while(!this->b_killThread){
		Sleep(100);
		if(reload_timer.HasTimedOut(60*60) || !b_has_loaded){  //once an hour we will search all over again

			if(mv_search_projects.Size()>0){  
				//we still have a list of titles that need searched
				continue;
			}

			if(!m_last_search.HasTimedOut(3*60)){
				//wait until we have stopped searching for a couple minutes
				continue;
			}

			if(m_num_search_results>0){
				LogSearchResults();
				for(int i=0;i<NUMCONBINS;i++)
					mv_search_results[i].Clear();
			}

			m_num_search_results=0;

			reload_timer.Refresh();
			b_has_loaded=true;

			SystemRef ref;
			ref.System()->Log("Project Cache:  Querying database server for new project information...");
			TinySQL sql;
			if(!sql.Init("63.221.232.36","onsystems","ebertsux37","project_management",3306)){
				//ref.System()->Log("Project Cache:  Failed to init database connection, will retry in 1 hour.");
				continue;
			}

			if(!sql.Query("select project_name,owner,artist,search_string,id,size_threashold,album from projects where active='A' order by rand()",true)){
				ref.System()->Log("Project Cache:  the project query failed, will retry in 1 hour.");
				continue;  //don't clear our old projects if we can't get any new ones right now
			}

			Vector v_tmp_projects;
			
			for(int i=0;i<(int)sql.m_num_rows;i++){
				CString project=sql.mpp_results[i][0].c_str();
				CString artist=sql.mpp_results[i][2].c_str();
				CString owner=sql.mpp_results[i][1].c_str();
				CString search_string=sql.mpp_results[i][3].c_str();
				UINT id=atoi(sql.mpp_results[i][4].c_str());
				UINT size_threashold=atoi(sql.mpp_results[i][5].c_str());
				CString album=sql.mpp_results[i][6].c_str();
				

				search_string=search_string.MakeLower();
				search_string.Replace("mp3","");
				search_string=search_string.Trim();

				//mv_projects
				v_tmp_projects.Add(new AresDCProjectInfo(project,search_string,owner,artist,id,size_threashold,album));
			}

			CString log_msg;
			log_msg.Format("Project Cache:  Loaded %u projects relevant to Ares",v_tmp_projects.Size());
			ref.System()->Log(log_msg);
			ref.System()->Log("Project Cache:  Setting track info for each project...");


			//gather track listings
			if(!sql.Query("select track_listing.project_id,track_listing.track_name,track_listing.track_number from track_listing,projects where track_listing.project_id=projects.id AND projects.active='A'",true)){
				continue;
			}

			vector<UINT> v_track_proj_id;
			vector<string> v_track_track_name;
			vector<UINT> v_track_track_number;


			for(int i=0;i<(int)sql.m_num_rows;i++){
				UINT id=atoi(sql.mpp_results[i][0].c_str());
				string track_name=sql.mpp_results[i][1].c_str();
				int track_number=atoi(sql.mpp_results[i][2].c_str());

				for(int j=0;j<(int)v_tmp_projects.Size();j++){
					AresDCProjectInfo *proj_info=(AresDCProjectInfo*)v_tmp_projects.Get(j);
					if(proj_info->m_id==id){
						proj_info->AddTrack(track_name.c_str(),track_number,false);
					}
				}
			}

			//all tracks have been added, now we need to get their keywords and killwords
			//gather keywords
			if(!sql.Query("select keywords.project_id,keywords.track_number,keywords.keyword from keywords,projects where keywords.project_id=projects.id AND projects.active='A'",true)){
				continue;
			}
			for(int i=0;i<(int)sql.m_num_rows;i++){
				UINT id=atoi(sql.mpp_results[i][0].c_str());
				int track_number=atoi(sql.mpp_results[i][1].c_str());
				CString keyword=sql.mpp_results[i][2].c_str();
				keyword=keyword.MakeLower();

				for(int j=0;j<(int)v_tmp_projects.Size();j++){
					AresDCProjectInfo *proj_info=(AresDCProjectInfo*)v_tmp_projects.Get(j);
					if(proj_info->m_id==id){
						proj_info->AddTrackKeyword(keyword,track_number);
					}
				}
			}

			//all tracks have been added, now we need to get their keywords and killwords
			//gather keywords
			if(!sql.Query("select killwords.project_id,killwords.killword from killwords,projects where killwords.project_id=projects.id AND projects.active='A'",true)){
				continue;
			}
			for(int i=0;i<(int)sql.m_num_rows;i++){
				UINT id=atoi(sql.mpp_results[i][0].c_str());
				CString killword=sql.mpp_results[i][1].c_str();
				killword=killword.MakeLower();

				for(int j=0;j<(int)v_tmp_projects.Size();j++){
					AresDCProjectInfo *proj_info=(AresDCProjectInfo*)v_tmp_projects.Get(j);
					if(proj_info->m_id==id){
						proj_info->AddKillword(killword);
					}
				}
			}

			//this data collector must function differently than most data collectors.  it must search for each track individually
			//now we need to go through each of these projects and create a search job for each track, if one exists

			//Vector v_search_jobs;

			int added_jobs=0;

			for(int proj_index=0;proj_index<(int)v_tmp_projects.Size();proj_index++){
				AresDCProjectInfo *proj_info=(AresDCProjectInfo*)v_tmp_projects.Get(proj_index);
				
				bool b_has_tracks=false;
				CString proj_search_str=proj_info->m_search_string.c_str();
				proj_search_str=proj_search_str.Trim();

				CString album=proj_info->m_album.c_str();
				CString artist=proj_info->m_artist.c_str();
				album=album.Trim();
				artist=artist.Trim();
				if(album.GetLength()<1)
					album=proj_info->m_name.c_str();

				if(artist.GetLength()<1)
					artist=proj_info->m_name.c_str();

				for(int track_index=0;track_index<(int)proj_info->mv_tracks.Size();track_index++){
					
					b_has_tracks=true;
					CString tmp_search_str=proj_search_str;
					
					AresDCTrackInfo *track_info=(AresDCTrackInfo*)proj_info->mv_tracks.Get(track_index);
					for(int j=0;j<(int)track_info->mv_keywords.size();j++){
						tmp_search_str+=" ";
						tmp_search_str+=track_info->mv_keywords[j].c_str();
					}

					
					SearchJob *new_search_job=new SearchJob(proj_info->m_id,proj_info->m_artist.c_str(),album,track_info->m_track_number,track_info->m_name.c_str(),tmp_search_str,proj_info->m_size_threashold);
					for(int j=0;j<(int)proj_info->mv_killwords.size();j++){
						new_search_job->AddKillword(proj_info->mv_killwords[j].c_str());
					}
					if(new_search_job->mb_valid){
						mv_search_projects.Add(new_search_job);
						mv_search_results[new_search_job->m_search_id%NUMCONBINS].Add(new_search_job);
						m_num_search_results++;
						added_jobs++;
					}
					else delete new_search_job;
				}
				if(!b_has_tracks){
					SearchJob *new_search_job=new SearchJob(proj_info->m_id,artist,album,0,"",proj_search_str,proj_info->m_size_threashold);
					for(int j=0;j<(int)proj_info->mv_killwords.size();j++){
						new_search_job->AddKillword(proj_info->mv_killwords[j].c_str());
					}
					if(new_search_job->mb_valid){
						mv_search_projects.Add(new_search_job);
						mv_search_results[new_search_job->m_search_id%NUMCONBINS].Add(new_search_job);
						m_num_search_results++;
						added_jobs++;
					}
					else delete new_search_job;
				}
			}

			//v_tmp_projects.Sort(1);

			log_msg.Format("SearchManager:  Added %u search jobs...",added_jobs);
			ref.System()->Log(log_msg);

			//CSingleLock lock(&m_lock,TRUE);
			//mv_projects.Copy(&v_tmp_projects);

			
		}
	}
	return 0;
}

void SearchManager::GetNextSearchJob(Vector &v)
{
	if(mv_search_projects.Size()<1)
		return;
	else{
		m_last_search.Refresh();
		v.Add(mv_search_projects.Get(0));
		mv_search_projects.Remove(0);
	}
}

void SearchManager::PostSearchResult(UINT query_id, const char* file_name, const char* artist, const char* album, const char* title, const char* user_name,const char* ip, byte* hash,UINT file_size)
{
	TRACE("Received Search Result:\n");
	TRACE("file_name = %s\n",file_name);
	TRACE("artist = %s\n",artist);
	TRACE("album = %s\n",album);
	TRACE("title = %s\n",title);
	TRACE("username = %s\n",user_name);
	TRACE("ip = %s\n",ip);
	TRACE("query_id = %u\n",query_id);
	TRACE("\n");
	CString chash=EncodeBase16(hash,20);

	for(int i=0;i<(int)mv_search_results[query_id%NUMCONBINS].Size();i++){
		SearchJob* job=(SearchJob*)mv_search_results[query_id%NUMCONBINS].Get(i);
		if(job->m_search_id==query_id){

			//is the file to small to count for this title?
			if(file_size<job->m_size_threashold)
				return;

			if(job->m_size_threashold<3000000 && file_size>50000000){
				//ignore music videos
				return;
			}

			
			//see if this search result contains any killswords
			for(int j=0;j<(int)job->mv_killwords.size();j++){
				char* kw=(char*)job->mv_killwords[j].c_str();

				CString cartist=artist;
				CString calbum=album;
				CString cfile_name=file_name;
				CString ctitle=title;
				

				cartist=cartist.MakeLower();
				calbum=calbum.MakeLower();
				cfile_name=cfile_name.MakeLower();
				ctitle=ctitle.MakeLower();

				if(cartist.Find(kw)!=-1)
					return;
				if(calbum.Find(kw)!=-1)
					return;
				if(cfile_name.Find(kw)!=-1)
					return;
				if(ctitle.Find(kw)!=-1)
					return;
			}


			job->mv_supply_hashes.push_back((LPCSTR)chash);
			job->mv_supply_ips.push_back(ip);
		}
	}
}

void SearchManager::LogSearchResults(void)
{
	SystemRef ref;
	ref.System()->GetSearchProcessor()->AddResults(mv_search_results);
}

