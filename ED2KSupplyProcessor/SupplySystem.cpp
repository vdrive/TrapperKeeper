#include "StdAfx.h"
#include "supplysystem.h"
#include "TinySQL.h"
#include "Timer.h"

UINT SupplySystem::sm_system_count=0;
SupplySystem* SupplySystem::sm_system=NULL;
CCriticalSection SupplySystem::sm_system_lock;

SupplySystem::SupplySystem(void)
{
}

SupplySystem::~SupplySystem(void)
{
}

UINT SupplySystem::Run(void)
{
	for(int i=0;i<10 && !this->b_killThread;i++){
		Sleep(1000);
	}
	TRACE("SupplySystem::Run(void)  Supply Processing Thread Active\n");
	Timer process_timer;
	while(!this->b_killThread){
		Sleep(500);
		if(process_timer.HasTimedOut(30) && m_project_manager.IsReady()){  //every 5 minutes we process our supply
			process_timer.Refresh();
			TRACE("SupplySystem::Run(void) Processing raw_search_results BEGIN.\n");
			Vector projects;
			//ProjectKeywordsVector projects;
			m_project_manager.GetProjectsCopy(projects);
			TinySQL supply_sql;
			supply_sql.Init("38.119.64.66","onsystems","tacobell","ed2ksupply",3306);

			TinySQL sql;
			sql.Init("38.119.64.66","onsystems","tacobell","metamachine",3306);

			//retrieve a list of files that we have cross named into another project, we don't want to count those as supply for that project
			sql.Query("select project,hash from anti_leak_cross_named order by 1",true);

			int last_project_index=0;

			for(int i=0;i<(int)sql.m_num_rows;i++){  //for each cross named hash
				for(int j=0;j<(int)projects.Size();j++){  //find the corresponding project
					int pindex=(j+last_project_index)%projects.Size();
					ED2KProjectInfo *pi=(ED2KProjectInfo*)projects.Get(pindex);
					if(stricmp(pi->m_project_keywords.m_project_name.c_str(),sql.mpp_results[i][0].c_str())==0){
						//add this hash to that project
						pi->mv_cross_name_hashes.push_back(sql.mpp_results[i][1].c_str());
						last_project_index=pindex;
						break;
					}
				}
			}

			//we need to gather anti leak hashes because those are being cross named as well, we don't want to count one projects supply as anothers
			sql.Query("select project,hash from anti_leak_table",true);

			last_project_index=0;


			for(int i=0;i<(int)sql.m_num_rows;i++){ //for each hash marked for anti leak protection
				for(int j=0;j<(int)projects.Size();j++){  //find the corresponding project
					int pindex=(j+last_project_index)%projects.Size();
					ED2KProjectInfo *pi=(ED2KProjectInfo*)projects.Get(pindex);
					if(stricmp(pi->m_project_keywords.m_project_name.c_str(),sql.mpp_results[i][0].c_str())==0){
						//add this hash to that project
						pi->mv_anti_leak_hashes.push_back(sql.mpp_results[i][1].c_str());
						last_project_index=pindex;
						break;
					}
				}
			}

			//using the same sql connections, pump some raw supply data
			for(int loop_index=0;loop_index<20 && !this->b_killThread;loop_index++){
				Sleep(300);

				//lock the tables to prevent any changes
				TRACE("SupplySystem::Run(void) Processing raw_search_results QUERYING.\n");
				//process supply chunks of xK
				sql.Query("select project,file_name,file_size,hash,availability,server_ip,time_created from raw_search_results limit 5000",true);
				
				TRACE("SupplySystem::Run(void) Processing raw_search_results PROCESSING %d results.\n",sql.m_num_rows);
				vector <string> v_queries;
				vector <string> v_cross_name_queries;
				ED2KProjectInfo *cur_project=NULL;  //remember last project that we looked up, to save cpu
				int num_results=(int)sql.m_num_rows;
				if(num_results==0)
					break;
				for( int result_index=0 ; result_index < (int)sql.m_num_rows ; result_index++ ){
					string project=sql.mpp_results[result_index][0].c_str();
					string file_name=sql.mpp_results[result_index][1].c_str();
					string avail=sql.mpp_results[result_index][4].c_str();
					string hash=sql.mpp_results[result_index][3].c_str();
					string server_ip=sql.mpp_results[result_index][5].c_str();
					string time_created=sql.mpp_results[result_index][6].c_str();
					UINT file_size=atoi(sql.mpp_results[result_index][2].c_str());

					if(file_size<100000)
						continue;

					if(cur_project!=NULL && stricmp(cur_project->m_project_keywords.m_project_name.c_str(),project.c_str())==0){
						ProcessSearchResult(cur_project,file_name,avail,hash,server_ip,time_created,file_size,v_queries,v_cross_name_queries);
					}
					else{
						for(int i=0;i<(int)projects.Size();i++){
							ED2KProjectInfo *pi=(ED2KProjectInfo*)projects.Get(i);
							if(pi->m_project_keywords.m_project_active && stricmp(pi->m_project_keywords.m_project_name.c_str(),project.c_str())==0){
								cur_project=pi;//&projects.v_projects[i];
								string table_name=GetProjectTableName(cur_project->m_project_keywords.m_project_name.c_str());
								CString query;
								//make sure a table for this project exists
								query.Format("create table %s (track INT NOT NULL,time DATE NOT NULL,availability INT NOT NULL,size INT NOT NULL,hash varchar(33) NOT NULL,file_name varchar(255) NOT NULL,server_ip varchar(255) NOT NULL,process_state INT(3) NOT NULL,primary key(time,track,hash,server_ip))",table_name.c_str());
								supply_sql.Query(query,false);
								ProcessSearchResult(cur_project,file_name,avail,hash,server_ip,time_created,file_size,v_queries,v_cross_name_queries);
								break;
							}
						}
					}
				}
				TRACE("SupplySystem::Run(void) Processing raw_search_results using %d out of %d raw search results.\n",v_queries.size()/2,num_results);

				TRACE("SupplySystem::Run(void) Processing raw_search_results CLEANING.\n");
				CString delete_query;
				delete_query.Format("delete from raw_search_results limit %d",num_results);
				sql.Query(delete_query,false);
				
				/*
				CString insert_query="insert ignore into rawsupply2 values ";
				for(int query_index=0;query_index<(int)v_queries.size();query_index++){
					CString tmp;
					if(query_index!=0)
						tmp=",";
					tmp+=v_queries[query_index].c_str();
					insert_query+=tmp;	
				}*/
				
				//if(v_queries.size()>0)
				//	supply_sql.Query(insert_query,false);
				for(int query_index=0;query_index<(int)v_queries.size();query_index++){
					supply_sql.Query(v_queries[query_index].c_str(),false);
				}

				for(int query_index=0;query_index<(int)v_cross_name_queries.size();query_index++){
					sql.Query(v_cross_name_queries[query_index].c_str(),false);
				}
			}
			//process queries
			process_timer.Refresh();
			
			TRACE("SupplySystem::Run(void) Processing raw_search_results END.\n");
		}
	}
	return 0;
}

void SupplySystem::StartSystem(Dll *p_dll)
{
	this->StartThread();
	m_project_manager.DllStart(p_dll,"ED2KSupplyProcessor");
}

void SupplySystem::StopSystem(void)
{
	TRACE("SupplySystem::StopSystem(void)\n");
	this->StopThread();
}

void SupplySystem::ProcessRawSupply(void)
{
}

void SupplySystem::ProcessSearchResult(ED2KProjectInfo* pi, string& file_name, string& availability, string& hash, string& server_ip, string& time_created, UINT file_size , vector<string> &v_queries,vector<string> &v_cross_name_queries)
{
	//see if the keywords match
	ProjectKeywords &project=pi->m_project_keywords;
	if(!project.m_emule_supply_enabled || !project.m_project_active)
		return;
	
	string lc_file_name=file_name;

	//make file_name lower case
	for(int i=0;i<(int)lc_file_name.size();i++){
		lc_file_name[i]=tolower(lc_file_name[i]);
	}

	bool b_movie=false;
	if(project.m_supply_keywords.v_keywords.size()==0)
		b_movie=true;

	if(b_movie){
		//this is a movie, so check kill words and log it as supply if none match
		for(int killword_index=0;killword_index<(int)project.m_supply_keywords.v_killwords.size();killword_index++){
			SupplyKeyword *killword=&project.m_supply_keywords.v_killwords[killword_index];
			for(int j=0;j<(int)killword->v_keywords.size();j++){
				if(strstr(lc_file_name.c_str(),killword->v_keywords[j])!=NULL){
					return;  //file_name contains a killword for this project
				}
			}
		}

		//see if we have cross named this hash into this project (if the supply is an illusion created by us)
		bool b_cross_named=false;

		if(!b_cross_named){  //if not designated as cross named, run an additional check
			for(int j=0;j<(int)pi->mv_anti_leak_hashes.size();j++){
				if(stricmp(pi->mv_anti_leak_hashes[j].c_str(),hash.c_str())==0){  
					//this hash has been designated for leak protection... see if it was on another project
					//does this hash actually belong to another project and due to cross naming its shown up int his project
					//yes this hash is really supply for a different project
					b_cross_named=true;
					break;
				}
			}
		}

		if(!b_cross_named){  //if not designated as cross named, run an additional check
			for(int i=0;i<(int)pi->mv_cross_name_hashes.size();i++){
				//see if the hash matches this one
				if(stricmp(pi->mv_cross_name_hashes[i].c_str(),hash.c_str())==0){
					b_cross_named=true;
					break;					
				}
			}
		}

		if(b_cross_named)
			return;  //don't count this as supply for this project, even though it appears to be.  Its a lie.

		CString query;
		CString tmp_file_name=file_name.c_str();
		PrepareStringForDatabase(tmp_file_name);

		string table_name=GetProjectTableName(project.m_project_name.c_str());

//		query.Format("update %s set availability=%s where hash='%s' and server_ip='%s' and time='%s' and processed=0",table_name.c_str(),availability.c_str(),hash.c_str(),server_ip.c_str(),time_created.c_str());
//		v_queries.push_back((LPCSTR)query);
		query.Format("insert ignore into %s values (0,'%s',%s,%u,'%s','%s','%s',0)",table_name.c_str(),time_created.c_str(),availability.c_str(),file_size,hash.c_str(),tmp_file_name,server_ip.c_str());

		//query.Format("('%s',0,'%s',%s,%d,'%s','%s','%s',0)",project.m_project_name.c_str(),time_created.c_str(),availability.c_str(),file_size,hash.c_str(),tmp_file_name,server_ip.c_str());
		//query.Format("insert ignore into rawsupply2 values (0,'%s',%s,%d,'%s','%s','%s',0)",table_name.c_str(),time_created.c_str(),availability.c_str(),file_size,hash.c_str(),tmp_file_name,server_ip.c_str());
		v_queries.push_back((LPCSTR)query);
		
		return;
	}
	else{
		//not a movie and has actual tracks we need to worry about
		for(int keyword_index=0;keyword_index<(int)project.m_supply_keywords.v_keywords.size();keyword_index++){
			SupplyKeyword *keyword=&project.m_supply_keywords.v_keywords[keyword_index];
			int track=keyword->m_track;
			bool b_has_all=true;
			for(int j=0;j<(int)keyword->v_keywords.size();j++){
				if(strstr(lc_file_name.c_str(),keyword->v_keywords[j])==NULL){
					b_has_all=false;
					break;  //file_name contains a killword for this project
				}
			}
			if(b_has_all){
				//check killwords for this track
				bool b_has_kill_words=false;
				for(int killword_index=0;killword_index<(int)project.m_supply_keywords.v_killwords.size() && !b_has_kill_words;killword_index++){
					SupplyKeyword *killword=&project.m_supply_keywords.v_killwords[killword_index];
					for(int j=0;j<(int)killword->v_keywords.size();j++){
						if(killword->m_track==track && strstr(lc_file_name.c_str(),killword->v_keywords[j])!=NULL){
							b_has_kill_words=true;
							break;  //file_name contains a killword for this track
						}
					}
				}
				if(b_has_kill_words)
					continue;

				//see if we have cross named this hash into this project (if the supply is an illusion created by us)
				bool b_cross_named=false;

				if(!b_cross_named){  //if not designated as cross named, run an additional check
					for(int j=0;j<(int)pi->mv_anti_leak_hashes.size();j++){
						if(stricmp(pi->mv_anti_leak_hashes[j].c_str(),hash.c_str())==0){  
							//this hash has been designated for leak protection... see if it was on another project
							//does this hash actually belong to another project and due to cross naming its shown up int his project
							//yes this hash is really supply for a different project
							b_cross_named=true;
							break;
						}
					}
				}

				if(!b_cross_named){  //if not designated as cross named, run an additional check
					for(int i=0;i<(int)pi->mv_cross_name_hashes.size();i++){
						//see if the hash matches this one
						if(stricmp(pi->mv_cross_name_hashes[i].c_str(),hash.c_str())==0){
							b_cross_named=true;
							break;					
						}
					}
				}

				if(b_cross_named)
					return;  //don't count this as supply for this project, even though it appears to be.  Its a lie.

				CString query;
				CString tmp_file_name=file_name.c_str();
				PrepareStringForDatabase(tmp_file_name);
				string table_name=GetProjectTableName(project.m_project_name.c_str());
//				query.Format("update %s set availability=%s where hash='%s' and server_ip='%s' and time='%s' and processed=0",table_name.c_str(),availability.c_str(),hash.c_str(),server_ip.c_str(),time_created.c_str());
//				v_queries.push_back((LPCSTR)query);

				query.Format("insert ignore into %s values (%d,'%s',%s,%u,'%s','%s','%s',0)",table_name.c_str(),track,time_created.c_str(),availability.c_str(),file_size,hash.c_str(),tmp_file_name,server_ip.c_str());
				//v_queries.push_back((LPCSTR)query);
				//query.Format("('%s',%d,'%s',%s,%d,'%s','%s','%s',0)",project.m_project_name.c_str(),track,time_created.c_str(),availability.c_str(),file_size,hash.c_str(),tmp_file_name,server_ip.c_str());
				//TRACE("Generated Query: %s\n",query);
				v_queries.push_back((LPCSTR)query);
				return;
			}
		}

		//this result was returned with our search string, yet it doesn't apply to this project
		int avail=atoi(availability.c_str());
		if(avail>10 && file_size>1000000){  //significantly large hashes should go into a source table for cross naming reference
			CString tmp_file_name=file_name.c_str();
			PrepareStringForDatabase(tmp_file_name);
			CString query;
			query.Format("insert ignore into anti_leak_table_cross_name_sources values ('%s','%s','%s',%u,now())",project.m_project_name.c_str(),tmp_file_name,hash.c_str(),file_size);
			v_cross_name_queries.push_back((LPCSTR)query);
		}
	}
}

void SupplySystem::PrepareStringForDatabase(CString &str)
{
	str.Replace("\\","\\\\");
	str.Replace("'","\\'");
}

string SupplySystem::GetProjectTableName(const char* project_name)
{
	int len=(int)strlen(project_name);
	string new_name;
	for(int i=0;i<len && i<60;i++){
		if(isdigit(project_name[i]) || isalpha(project_name[i])){
			new_name+=project_name[i];
		}
		else{
			new_name+="_";
		}
	}
	return new_name;
}
