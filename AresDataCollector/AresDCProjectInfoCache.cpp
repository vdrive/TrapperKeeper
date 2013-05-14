#include "StdAfx.h"
#include "aresdcprojectinfocache.h"
#include "AresDCProjectInfo.h"
#include "..\tkcom\timer.h"
#include "..\tkcom\TinySQL.h"
#include "AresDCTrackInfo.h"

#include "AresDataCollectorSystem.h"

AresDCProjectInfoCache::AresDCProjectInfoCache(void)
{
	mb_ready=false;

	m_search_index=0;
}

AresDCProjectInfoCache::~AresDCProjectInfoCache(void)
{
}


UINT AresDCProjectInfoCache::Run(){
	srand(GetTickCount());

	Timer startup;

	AresDataCollectorSystemReference ref;
	ref.System()->Log("Project Cache:  Project Loading Thread Started...");

	Timer last_update;
	bool b_has_updated=false;
	Timer last_search_process;
	Timer last_swarm_process;

	bool b_has_built_swarms=false;

	while(!this->b_killThread){
		Sleep(100);
	
		if(last_update.HasTimedOut(60*60*12) || !b_has_updated || (mv_projects.Size()==0 && last_update.HasTimedOut(60*60*1))){  //TYDEBUG
			last_update.Refresh();
			b_has_updated=true;
			ref.System()->Log("Project Cache:  Querying database server for new project information...");
			TinySQL sql;
			if(!sql.Init("63.221.232.36","onsystems","ebertsux37","project_management",3306)){
				ref.System()->Log("Project Cache:  Failed to init database connection, will retry in 1 hour.");
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
				CString album=sql.mpp_results[i][6].c_str();
				UINT id=atoi(sql.mpp_results[i][4].c_str());
				UINT size_threashold=atoi(sql.mpp_results[i][5].c_str());
				

				search_string=search_string.MakeLower();
				search_string.Replace("mp3","");
				search_string=search_string.Trim();

				//mv_projects
				v_tmp_projects.Add(new AresDCProjectInfo(project,search_string,owner,artist,id,size_threashold,album));
			}
			//CSingleLock lock(&m_lock,TRUE);
			//mv_projects.Clear();
			//mv_projects.Copy(&v_tmp);

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

			for(int j=0;j<(int)v_tmp_projects.Size();j++){
				AresDCProjectInfo *proj_info=(AresDCProjectInfo*)v_tmp_projects.Get(j);
				bool b_found=false;
				for(int i=0;i<(int)v_tmp_ids.size();i++){
					if(proj_info->m_id==v_tmp_ids[i]){
						if(vb_swarm[i])
							proj_info->mb_swarming=true;
						if(vb_decoy[i])
							proj_info->mb_decoying=true;
						b_found=true;
					}
				}
				if(!b_found){
					v_tmp_projects.Remove(j);  //not doing ares protection
					j--;
				}
			}

			//now all projects remaining in our vector are relevant to the ares system
			CString log_msg;
			log_msg.Format("Project Cache:  Loaded %u projects relevant to Ares",v_tmp_projects.Size());
			ref.System()->Log(log_msg);
			ref.System()->Log("Project Cache:  Setting track info for each project...");


			//gather track listings
			if(!sql.Query("select track_listing.project_id,track_listing.track_name,track_listing.track_number,track_listing.single from track_listing,projects where track_listing.project_id=projects.id AND projects.active='A'",true)){
				continue;
			}

			for(int i=0;i<(int)sql.m_num_rows;i++){
				UINT id=atoi(sql.mpp_results[i][0].c_str());
				string track_name=sql.mpp_results[i][1].c_str();
				int track_number=atoi(sql.mpp_results[i][2].c_str());
				string single=sql.mpp_results[i][3].c_str();
				bool b_single=false;
				if(stricmp(single.c_str(),"T")==0)
					b_single=true;

				for(int j=0;j<(int)v_tmp_projects.Size();j++){
					AresDCProjectInfo *proj_info=(AresDCProjectInfo*)v_tmp_projects.Get(j);
					if(proj_info->m_id==id){
						proj_info->AddTrack(track_name.c_str(),track_number,b_single);
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

			TRACE("AresDCProjectInfoCache::Run() Loaded %d projects\n",v_tmp_projects.Size());

			v_tmp_projects.Sort(1);

			CSingleLock lock(&m_lock,TRUE);
			mv_projects.Copy(&v_tmp_projects);

			ref.System()->Log("Project Cache:  All relevant projects are completely loaded.  Projects are ready for use.");
			mb_ready=true;
		}
		
		if(last_search_process.HasTimedOut(10*60)){  //every X minutes we will insert some search results  TYDEBUG
			ref.System()->Log("Project Cache:  Processing Search Results...");
			ProcessSearchResults();
			ref.System()->Log("Project Cache:  Finished Processing Search Results");
			last_search_process.Refresh();
		}
	
		if((ref.System()->IsController() && last_swarm_process.HasTimedOut(45*60)) || (!b_has_built_swarms && ref.System()->IsController())){
			b_has_built_swarms=true;
			ref.System()->Log("Project Cache:  Cleaning Tables...");
			CleanTables();
			ref.System()->Log("Project Cache:  Finished Cleaning Tables");
			ref.System()->Log("Project Cache:  Building Decoys");
			BuildDecoys();
			ref.System()->Log("Project Cache:  Finished Building Decoys");
			ref.System()->Log("Project Cache:  Building Swarms");
			BuildSwarms();
			ref.System()->Log("Project Cache:  Finished Building Swarms");
			ref.System()->Log("Project Cache:  Building Exception Hashes...");
			BuildExceptionHashes();  //TYDEBUG
			ref.System()->Log("Project Cache:  Finished Building Exception Hashes");
			last_swarm_process.Refresh();
		}
	}
	
	return 0;
}

void AresDCProjectInfoCache::GetProjects(Vector& v)
{
	CSingleLock lock(&m_lock,TRUE);
	v.Copy(&mv_projects);
}

bool AresDCProjectInfoCache::IsReady(void)
{
	if(mv_projects.Size()<1)
		return false;
	else
		return mb_ready;
}


void AresDCProjectInfoCache::GetNextProjectForSearch(Vector& v_tmp)
{
	CSingleLock lock(&m_lock,TRUE);

	if(m_search_index>=(int)mv_projects.Size()){
		m_search_index=0;
	}

	if(m_search_index>=(int)mv_projects.Size()){
		return;
	}

	v_tmp.Add(mv_projects.Get(m_search_index++));
}

void AresDCProjectInfoCache::ProcessSearchResults(void)
{
	AresDataCollectorSystemReference ref;
	ref.System()->Log("Project Cache:  Processing Search Results started...");


	TinySQL regular_sql;
	
	if(!regular_sql.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306)){
		ref.System()->Log("Project Cache:  Failed to connect to database while processing search results");
		return;
	}

	TinySQL sql_supply;
	
	if(!sql_supply.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_supply",3306)){
		ref.System()->Log("Project Cache:  Failed to connect to database while processing search results");
		return;
	}

	static Timer m_last_clear;
	if(m_last_clear.HasTimedOut(60*60*2) && ref.System()->IsController()){
		regular_sql.Query("delete from ares_supply where time_created=(now()-INTERVAL 2 DAY)",false);
		regular_sql.Query("delete from ares_supply where time_created=(now()-INTERVAL 5 DAY)",false);
		regular_sql.Query("delete from ares_swarm_table where time_created<(now()-INTERVAL 2 DAY)",false);
		regular_sql.Query("delete from ares_decoys where time_created<(now()-INTERVAL 2 DAY)",false);
		regular_sql.Query("select count(*) from ares_hosts",true);
		if(regular_sql.m_num_rows>0){
			int count=atoi(regular_sql.mpp_results[0][0].c_str());
			if(count>60000)
				regular_sql.Query("delete from ares_hosts where time_created<(now()-INTERVAL 5 DAY) limit 5000",false);
		}
		m_last_clear.Refresh();
	}

	for(int project_index=0;project_index<(int)mv_projects.Size();project_index++){
		AresDCProjectInfo *proj_info=(AresDCProjectInfo*)mv_projects.Get(project_index);
		
		string table_name=GetProjectTableName(proj_info->m_name.c_str());
		while(proj_info->mv_search_results.Size()>0){
			AresSearchResult *result=(AresSearchResult*)proj_info->mv_search_results.Get(0);

			//is it an invalid size?
			if(result->m_file_size<proj_info->m_size_threashold || ((result->m_file_size%137)==0)){
				proj_info->mv_search_results.Remove(0);
				continue;
			}
			
			//does it match killwords?
			if(proj_info->DoesMatchKillwords(result->m_file_name.c_str(),result->m_artist.c_str(),result->m_title.c_str())){
				proj_info->mv_search_results.Remove(0);
				continue;
			}

			AresDCTrackInfo* ti=proj_info->DoesMatchSingle(result->m_file_name.c_str(),result->m_artist.c_str(),result->m_title.c_str());
		
			if(ti==NULL && proj_info->mv_tracks.Size()>0){
				proj_info->mv_search_results.Remove(0);
				continue;
			}

			if(proj_info->mv_tracks.Size()>0 && proj_info->m_size_threashold<5000000){
				//do we have a big file on what appears to be a music project?  its probably a music video
				if(result->m_file_size>20000000){  //don't do music videos for music projects
					proj_info->mv_search_results.Remove(0);
					continue;
				}
			}

			//if(proj_info->mv_tracks.size()==0){
			
			if(proj_info->mv_tracks.Size()==0 && proj_info->m_size_threashold>5000000){
				//make sure the search string is actually in the title and the file_name, so we can eliminate all the cross naming bs
				//we can only do this for movies, music will rarely have the search keyword in the title
				CString lc_file_name=result->m_file_name.c_str();
				lc_file_name=lc_file_name.MakeLower();

				CString lc_title=result->m_title.c_str();
				lc_title=lc_title.MakeLower();

				bool b_bad_result=false;

				for(int search_keyword_index=0;search_keyword_index<(int)proj_info->mv_search_keywords.size();search_keyword_index++){
					CString search_keyword=proj_info->mv_search_keywords[search_keyword_index].c_str();
					search_keyword=search_keyword.MakeLower();
					search_keyword=search_keyword.Trim();
					if(search_keyword.GetLength()<2)
						continue;
					if(lc_file_name.Find(search_keyword)==-1){
						b_bad_result=true;
						break;
					}
					if(lc_title.Find(search_keyword)==-1){
						b_bad_result=true;
						break;
					}
				}
				if(b_bad_result){
					proj_info->mv_search_results.Remove(0);
					continue;
				}
			}
			//}


 //project      | varchar(250) |      | PRI |            |
 //track        | int(11)      | YES  |     | NULL       |
 //file_name    | varchar(250) | YES  |     | NULL       |
 //artist       | varchar(250) | YES  |     | NULL       |
 //album        | varchar(250) | YES  |     | NULL       |
 //bitrate      | int(11)      | YES  |     | NULL       |
 //media_length | int(11)      | YES  |     | NULL       |
 //hash         | varchar(41)  |      | PRI |            |
 //time_created | date         |      | PRI | 0000-00-00 |
 //host         | int(11)      |      | PRI | 0          |

			CString query;
			CString file_name=result->m_file_name.c_str();
			CString artist=result->m_artist.c_str();
			CString album=result->m_album.c_str();
			CString comments=result->m_comments.c_str();
			CString title=result->m_title.c_str();
			CString hash=EncodeBase16(result->m_hash,20);
			CString genre=result->m_genre.c_str();
			CString user_name=result->m_user_name.c_str();

			PrepareStringForDatabase(file_name);
			PrepareStringForDatabase(artist);
			PrepareStringForDatabase(album);
			PrepareStringForDatabase(comments);
			PrepareStringForDatabase(title);
			PrepareStringForDatabase(genre);
			PrepareStringForDatabase(user_name);

			file_name=file_name.Trim();
			title=title.Trim();
			artist=artist.Trim();
			album=album.Trim();
			genre=genre.Trim();

			if(file_name.GetLength()<5){
				proj_info->mv_search_results.Remove(0);
				continue;
			}

			int track_num=0;
			if(ti!=NULL)
				track_num=ti->m_track_number;

			query.Format("insert ignore into %s values ('%s',%d,'%s','%s','%s','%s','%s','%s',%d,%d,'%s','%u',now(),%u,0)",table_name.c_str(),proj_info->m_name.c_str(),track_num,file_name,title,artist,album,genre,comments,result->m_bitrate,result->m_media_length,hash,result->m_file_size,result->m_host);
			if(!sql_supply.Query(query,false)){
				CString table_create_query;

//				project      | varchar(250) |      | PRI |
//				track        | int(11)      | YES  |     | NULL
//				file_name    | varchar(250) | YES  |     | NULL
//				title        | varchar(250) | YES  |     | NULL
//				artist       | varchar(250) | YES  |     | NULL
//				album        | varchar(250) | YES  |     | NULL
//				genre        | varchar(255) | YES  |     | NULL
//				comments     | varchar(255) | YES  |     | NULL
//				bitrate      | int(11)      | YES  |     | NULL
//				media_length | int(11)      | YES  |     | NULL
//				hash         | varchar(41)  |      | PRI |
//				size         | int(11)      | YES  |     | NULL
//				time_created | date         |      | PRI | 0000-00-00
//				host         | int(11)      |      | PRI | 0
//				amount       | int(11)      |      |     | 0

				CString log_msg;
				log_msg.Format("Project Cache:  Failed insert query because '%s'!!?",sql_supply.m_fail_reason.c_str());
				ref.System()->Log(log_msg);

				table_create_query.Format("create table %s (project varchar(250) NOT NULL,track INT,file_name varchar(250),title varchar(250),artist varchar(250),album varchar(250),genre varchar(250), comments varchar(250),bitrate INT,medialength INT,hash varchar(41) NOT NULL,size INT,time_created DATE NOT NULL,host INT NOT NULL,amount INT, primary key(project,hash,time_created,host))",
							table_name.c_str());
				if(!sql_supply.Query(table_create_query,false)){
					CString log_msg;
					log_msg.Format("Project Cache:  Failed table creation query because '%s'!!?",sql_supply.m_fail_reason.c_str());
					ref.System()->Log(log_msg);
				}
			}

			query.Format("update %s set amount=amount+1 where project='%s' and hash='%s' and size=%u and host=%u and time_created=now()",table_name.c_str(),proj_info->m_name.c_str(),hash,result->m_file_size,result->m_host);
			if(!sql_supply.Query(query,false)){
				CString log_msg;
				log_msg.Format("Project Cache:  Failed update query because '%s'!!?",sql_supply.m_fail_reason.c_str());
				ref.System()->Log(log_msg);
			}

			if(proj_info->mb_swarming || proj_info->mb_decoying){
				query.Format("insert ignore into ares_interdiction values ('%s','%s',%u,'%s',%u,'%s','%s',now())",proj_info->m_name.c_str(),hash,result->m_file_size,result->m_ip.c_str(),result->m_port,user_name,file_name);
				if(!regular_sql.Query(query,false)){
					CString log_msg;
					log_msg.Format("Project Cache:  Failed insert interdiction query because '%s'!!?",regular_sql.m_fail_reason.c_str());
					ref.System()->Log(log_msg);
				}
			}

			proj_info->mv_search_results.Remove(0);
		}
	}


	ref.System()->Log("Project Cache:  Finished processing search results");
}

void AresDCProjectInfoCache::BuildSwarms(void)
{
	AresDataCollectorSystemReference ref;
	ref.System()->Log("Project Cache:  Processing Search Results started...");

	TinySQL sql;
//	TinySQL sql2;
	TinySQL sql_inserter;
	
	if(!sql.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_supply",3306)){
		ref.System()->Log("Project Cache:  Failed to connect to database while building swarm table");
		return;
	}

	if(!sql_inserter.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306)){
		ref.System()->Log("Project Cache:  Failed to connect to database while building swarm table");
		return;
	}

	for(int project_index=0;project_index<(int)mv_projects.Size();project_index++){
		AresDCProjectInfo* proj_info=(AresDCProjectInfo*)mv_projects.Get(project_index);
		if(!proj_info->mb_swarming)
			continue;

		string table_name=GetProjectTableName(proj_info->m_name.c_str());
		CString query;
		query.Format("select project,track,hash,file_name,artist,title,album,comments,genre,bitrate,medialength,size,sum(amount) from %s where project='%s' group by hash",table_name.c_str(),proj_info->m_name.c_str());
		if(!sql.Query(query,true)){
			CString log_msg;
			log_msg.Format("Project Cache:  Failed supply query while building swarm table because '%s'!!?",sql.m_fail_reason.c_str());
			ref.System()->Log(log_msg);
			return;
		}

		int len=sql.m_num_rows;

		//we need to figure out the top 9 hashes on a per track basis
		Vector v_track_objects;
		for(int sql_index=0;sql_index<len;sql_index++){
			CString project=sql.mpp_results[sql_index][0].c_str();
			UINT track=atoi(sql.mpp_results[sql_index][1].c_str());
			CString hash=sql.mpp_results[sql_index][2].c_str();
			UINT amount=atoi(sql.mpp_results[sql_index][12].c_str());

			TrackHashCollection *the_track=NULL;
			for(int track_index=0;track_index<(int)v_track_objects.Size();track_index++){
				TrackHashCollection *thc=(TrackHashCollection*)v_track_objects.Get(track_index);
				if(thc->m_track==track){
					the_track=thc;  //we've already recorded data for this track
					break;
				}
			}
			//add this track if it doesn't exist
			if(the_track==NULL){
				the_track=new TrackHashCollection();
				the_track->m_track=track;
				v_track_objects.Add(the_track);
			}

			//is this amount greater than any of the first seven amounts we've recorded so far?
			//if so, then insert it.

			bool b_added_amount=false;

			//create a list of hashes sorted by amount for this track
			for(int hash_index=0;hash_index<(int)the_track->v_amounts.size() && hash_index<9;hash_index++){
				if(amount>the_track->v_amounts[hash_index]){
					the_track->v_amounts.insert(the_track->v_amounts.begin()+hash_index,amount);
					the_track->v_hashes.insert(the_track->v_hashes.begin()+hash_index,(LPCSTR)hash);
					b_added_amount=true;
					break;
				}
			}

			if(!b_added_amount && the_track->v_amounts.size()<9){
				the_track->v_amounts.push_back(amount);
				the_track->v_hashes.push_back((LPCSTR)hash);
			}
		}

		for(int sql_index=0;sql_index<len;sql_index++){
			CString project=sql.mpp_results[sql_index][0].c_str();
			UINT track=atoi(sql.mpp_results[sql_index][1].c_str());
			CString hash=sql.mpp_results[sql_index][2].c_str();
			CString file_name=sql.mpp_results[sql_index][3].c_str();
			CString artist=sql.mpp_results[sql_index][4].c_str();
			CString title=sql.mpp_results[sql_index][5].c_str();
			CString album=sql.mpp_results[sql_index][6].c_str();
			CString comments=sql.mpp_results[sql_index][7].c_str();
			CString genre=sql.mpp_results[sql_index][8].c_str();
			UINT bitrate=atoi(sql.mpp_results[sql_index][9].c_str());
			UINT media_length=atoi(sql.mpp_results[sql_index][10].c_str());
			UINT size=atoi(sql.mpp_results[sql_index][11].c_str());
			UINT amount=atoi(sql.mpp_results[sql_index][12].c_str());

			int priority=10;

			if(amount<5 || ((size%137)==0)){  //only make note of a significant supply, otherwise we will swarm our own propogated swarms
				continue;
			}

			PrepareStringForDatabase(project);
			PrepareStringForDatabase(file_name);
			PrepareStringForDatabase(artist);
			PrepareStringForDatabase(title);
			PrepareStringForDatabase(comments);
			PrepareStringForDatabase(genre);
			PrepareStringForDatabase(album);


			if(amount>600)
				priority=90;
			if(amount>400)
				priority=80;
			else if(amount>300)
				priority=70;
			else if(amount>180)
				priority=60;
			else if(amount>120)
				priority=50;
			else if(amount>80)
				priority=40;
			else if(amount>60)
				priority=35;
			else if(amount>35)
				priority=25;
			else if(amount>15)
				priority=15;

			if(hash.GetLength()<40 || file_name.GetLength()<3)  //invalid supply from somewhere
				continue;


			//we want to swarm the middle hashes the hardest
			for(int track_index=0;track_index<(int)v_track_objects.Size();track_index++){
				TrackHashCollection *thc=(TrackHashCollection*)v_track_objects.Get(track_index);
				for(int hash_index=2;hash_index<(int)thc->v_hashes.size() && hash_index<9;hash_index++){
					if(stricmp(hash,thc->v_hashes[hash_index].c_str())==0){
						priority=100;  
					}
				}
			}

			CString query;

			//add a swarm file
			query.Format("insert ignore into ares_swarm_table values ('%s',%u,'%s','%s','%s','%s','%s','%s',%u,%u,'%s',%u,%d,now())",project,track,file_name,title,artist,album,genre,comments,bitrate,media_length,hash,size,priority);
			if(!sql_inserter.Query(query,false)){
				CString log_msg;
				log_msg.Format("Project Cache (BuildSwarms):  Failed insert query on swarm table because '%s'!!?",sql.m_fail_reason.c_str());
				ref.System()->Log(log_msg);
			}

			query.Format("update ares_swarm_table set priority=%d where hash='%s'",priority,hash);
			if(!sql_inserter.Query(query,false)){
				CString log_msg;
				log_msg.Format("Project Cache (BuildSwarms):  Failed update query on swarm table because '%s'!!?",sql.m_fail_reason.c_str());
				ref.System()->Log(log_msg);
			}
		}
	}

	ref.System()->Log("Project Cache:  Finished building swarm table");
}

void AresDCProjectInfoCache::BuildExceptionHashes(void)
{
	AresDataCollectorSystemReference ref;
	ref.System()->Log("Project Cache:  Building exception hashes...");

	TinySQL sql;
	
	if(!sql.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306)){
		ref.System()->Log("Project Cache:  Failed to connect to database while building exception hashes");
		return;
	}

	//get rid of the most popular hashes
	for(int i=0;i<(int)mv_exception_hashes.size();i++){
		CString query;
		query.Format("delete from ares_interdiction where hash='%s'",mv_exception_hashes[i].c_str());
		sql.Query(query,false);
		query.Format("delete from ares_swarm_table where hash='%s'",mv_exception_hashes[i].c_str());
		sql.Query(query,false);
	}
	ref.System()->Log("Project Cache:  Finished building exception hashes...");

}

void AresDCProjectInfoCache::GetExceptionHashes(vector<string> &v_exception_hashes)
{
	CSingleLock lock(&m_lock,TRUE);	
	v_exception_hashes=mv_exception_hashes;
}

void AresDCProjectInfoCache::BuildDecoys(void)
{
	AresDataCollectorSystemReference ref;
	ref.System()->Log("Project Cache:  Processing Search Results started...");

	TinySQL sql;
//	TinySQL sql2;
	TinySQL sql_inserter;
	
	if(!sql.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_supply",3306)){
		ref.System()->Log("Project Cache:  Failed to connect to database while building decoy table");
		return;
	}

	if(!sql_inserter.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306)){
		ref.System()->Log("Project Cache:  Failed to connect to database while building decoy table");
		return;
	}

	for(int project_index=0;project_index<(int)mv_projects.Size();project_index++){
		AresDCProjectInfo* proj_info=(AresDCProjectInfo*)mv_projects.Get(project_index);
		if(!proj_info->mb_decoying)
			continue;
		string table_name=GetProjectTableName(proj_info->m_name.c_str());
		CString query;
		//CString tmp_proj_name=proj_info->m_name.c_str();
		//tmp_proj_name.MakeLower();
		//if(tmp_proj_name.Find("serenity")==-1)  //TYDEBUG
		//	continue;
		query.Format("select project,track,hash,file_name,artist,title,album,comments,genre,bitrate,medialength,size,sum(amount) from %s where project='%s' group by file_name",table_name.c_str(),proj_info->m_name.c_str());
		if(!sql.Query(query,true)){
			CString log_msg;
			log_msg.Format("Project Cache:  Failed supply query while building decoy table because '%s'!!?",sql.m_fail_reason.c_str());
			ref.System()->Log(log_msg);
			continue;
		}

		int len=sql.m_num_rows;
		for(int sql_index=0;sql_index<len;sql_index++){
			CString project=sql.mpp_results[sql_index][0].c_str();
			UINT track=atoi(sql.mpp_results[sql_index][1].c_str());
			CString hash=sql.mpp_results[sql_index][2].c_str();
			CString file_name=sql.mpp_results[sql_index][3].c_str();
			CString artist=sql.mpp_results[sql_index][4].c_str();
			CString title=sql.mpp_results[sql_index][5].c_str();
			CString album=sql.mpp_results[sql_index][6].c_str();
			CString comments=sql.mpp_results[sql_index][7].c_str();
			CString genre=sql.mpp_results[sql_index][8].c_str();
			UINT bitrate=atoi(sql.mpp_results[sql_index][9].c_str());
			UINT media_length=atoi(sql.mpp_results[sql_index][10].c_str());
			UINT size=atoi(sql.mpp_results[sql_index][11].c_str());
			UINT amount=atoi(sql.mpp_results[sql_index][12].c_str());

			//check to see if this track is still active
			bool b_found_track=false;
			for(int track_index=0;track_index<(int)proj_info->mv_tracks.Size();track_index++){
				AresDCTrackInfo *track_info=(AresDCTrackInfo*)proj_info->mv_tracks.Get(track_index);
				if(track_info->m_track_number==track)
					b_found_track=true;
			}


			if((!b_found_track && track!=0) || amount<4 || ((size%137)==0)){  //only make note of a significant supply, otherwise we will swarm our own propogated swarms
				continue;
			}

			PrepareStringForDatabase(project);
			PrepareStringForDatabase(file_name);
			PrepareStringForDatabase(artist);
			PrepareStringForDatabase(title);
			PrepareStringForDatabase(comments);
			PrepareStringForDatabase(genre);
			PrepareStringForDatabase(album);

			if(hash.GetLength()<40 || file_name.GetLength()<3)  //invalid supply from somewhere
				continue;

			//add a decoy
			query.Format("insert ignore into ares_decoys values ('%s',%u,'%s','%s','%s','%s','%s','%s',%u,%u,%u,%u,now())",project,track,file_name,title,artist,album,genre,comments,bitrate,media_length,size,amount);
			//TRACE("Doing decoy insert %s, %s , %s\n",project,title,file_name);
			if(!sql_inserter.Query(query,false)){
				CString log_msg;
				log_msg.Format("Project Cache (BuildSwarms):  Failed insert query on decoy table because '%s'!!?",sql.m_fail_reason.c_str());
				ref.System()->Log(log_msg);
			}
		}
	}

	//get rid of situations where we have far too many filenames for a particular title, like when a competitor is protecting it

	//3 parralel vectors to temporarily 
	vector<string> v_projects;
	vector<int>v_tracks;
	vector<int>v_counts;
	sql_inserter.Query("select project,track,count(*) from ares_decoys group by project,track",true);
	for(int i=0;i<(int)sql_inserter.m_num_rows;i++){
		v_projects.push_back(sql_inserter.mpp_results[i][0].c_str());
		v_tracks.push_back(atoi(sql_inserter.mpp_results[i][1].c_str()));
		v_counts.push_back(atoi(sql_inserter.mpp_results[i][2].c_str()));
	}

	for(int i=0;i<(int)v_projects.size();i++){
		const char* project=v_projects[i].c_str();
		int track=v_tracks[i];
		int count=v_counts[i];
		if(count>150){
			CString query;

			//get rid of low popularity stuff
			query.Format("delete from ares_decoys where project='%s' and track=%d and popularity<12",project,track);
			sql_inserter.Query(query,false);

			//figure out how much we still have left after doing that
			query.Format("select project,track,count(*) from ares_decoys where project='%s' and track='%d' group by project,track",project,track);
			sql_inserter.Query(query,true);
			if(sql_inserter.m_num_rows>0){
				count=atoi(sql_inserter.mpp_results[0][2].c_str());
				if(count>150){
					//still too much left, randomly delete some things from the table, we must trim it down
					query.Format("delete from ares_decoys where project='%s' and track='%d' limit %d",project,track,count-150);
					sql_inserter.Query(query,false);
				}
			}
		}
	}

	ref.System()->Log("Project Cache:  Finished building swarm table");
}

void AresDCProjectInfoCache::CleanTables(void)
{
	AresDataCollectorSystemReference ref;
	TinySQL sql;
	TinySQL sql_deleter;
	
	if(!sql.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306)){
		ref.System()->Log("Project Cache:  Failed to connect to database while cleaning tables");
		return;
	}

	if(!sql_deleter.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306)){
		ref.System()->Log("Project Cache:  Failed to connect to database while cleaning tables");
		return;
	}

	if(mv_projects.Size()>0){
		if(!sql.Query("select project from ares_decoys group by project",true)){
			CString log_msg;
			log_msg.Format("Project Cache (BuildSwarms):  Failed to clean tables because '%s'!!?",sql.m_fail_reason.c_str());
			ref.System()->Log(log_msg);
		}
		for(int i=0;i<(int)sql.m_num_rows;i++){
			string project=sql.mpp_results[i][0].c_str();
			bool b_found=false;
			for(int j=0;j<(int)mv_projects.Size();j++){
				AresDCProjectInfo* proj_info=(AresDCProjectInfo*)mv_projects.Get(j);
				if(proj_info->mb_decoying && stricmp(proj_info->m_name.c_str(),project.c_str())==0){
					b_found=true;
					break;
				}
			}
			if(!b_found){
				CString query;
				query.Format("delete from ares_decoys where project='%s'",project.c_str());
				if(!sql_deleter.Query(query,false)){
					CString log_msg;
					log_msg.Format("Project Cache (BuildSwarms):  Failed to run delete query while cleaning tables because '%s'!!?",sql.m_fail_reason.c_str());
					ref.System()->Log(log_msg);
				}
			}
		}
	}
}
