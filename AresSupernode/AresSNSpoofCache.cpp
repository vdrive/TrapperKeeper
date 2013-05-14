#include "StdAfx.h"
#include "aressnspoofcache.h"
#include "..\tkcom\timer.h"
#include "..\tkcom\tinysql.h"
#include "AresSNSpoof.h"
#include "AresSupernodeSystem.h"
#include "SNKeyword.h"

AresSNSpoofCache::AresSNSpoofCache()
{


}

AresSNSpoofCache::~AresSNSpoofCache(void)
{
}

UINT AresSNSpoofCache::Run(void)
{
	Timer last_reload;
	bool b_has_loaded=false;
	srand(GetTickCount());
	int delay=10+(rand()%LOADDELAY);  //TYDEBUG
	while(!this->b_killThread){
		Sleep(100);

		if((!b_has_loaded && last_reload.HasTimedOut(delay)) || (last_reload.HasTimedOut(60*60*16) || (mv_spoofs.Size()==0 && last_reload.HasTimedOut(60*60*2)))){

			AresSupernodeSystemRef ref;
			ref.System()->LogToFile("AresSNSpoofCache::Run() BEGIN");

			b_has_loaded=true;
			last_reload.Refresh();
			TinySQL sql;
			
			CString log_msg;

			if(!sql.Init("206.161.141.35","onsystems","ebertsux37","ares_data",3306)){
				ref.System()->LogQuery("FAILED to init database connection for decoy/spoof cache.");
				ref.System()->LogToFile("AresSNSpoofCache::Run() END 0");
				continue;
			}

			if(!sql.Query("select count(*) from ares_decoys",true)){  //TYDEBUG
				log_msg.Format("FAILED to query database for decoy/spoofs because %s. (will retry later) ",sql.m_fail_reason.c_str());
				ref.System()->LogQuery(log_msg);
				ref.System()->LogToFile("AresSNSpoofCache::Run() END 1");
				continue;
			}

			if(sql.m_num_rows<1){
				log_msg.Format("FAILED to query database for decoy/spoofs because Corrupt Count. (will retry later) ");
				ref.System()->LogQuery(log_msg);
				ref.System()->LogToFile("AresSNSpoofCache::Run() END 1");
				continue;
			}

			int count=atoi(sql.mpp_results[0][0].c_str());

			int offset=rand()%(max(1,max(1,count-(NUMSPOOFS/2))/100));
			offset*=100;  //offset was scaled down by 100 because rand() only goes up to 65k
		
			offset=max(0,offset-(NUMSPOOFS/2)); //we do this so that we have a good chance of selecting the ones in front and the back, since we are randoming for a range.  If we didn't do this there would be about 1/150000 chance that the start would be picked, not good
			offset=min(offset,max(0,count-NUMSPOOFS));

			CString query;
			query.Format("select project,track,file_name,title,artist,album,genre,comments,bitrate,media_length,size,popularity from ares_decoys limit %u,%u",offset,NUMSPOOFS);

			if(!sql.Query(query,true)){  //TYDEBUG
				log_msg.Format("FAILED to query database for decoy/spoofs because %s. (will retry later) ",sql.m_fail_reason.c_str());
				ref.System()->LogQuery(log_msg);
				ref.System()->LogToFile("AresSNSpoofCache::Run() END 1");
				continue;
			}


			log_msg.Format("Queried %u (start_offset=%u) decoys/spoofs from database, adding to system now...",sql.m_num_rows,offset);
			ref.System()->LogQuery(log_msg);

			ref.System()->LogToFile("AresSNSpoofCache::Run() Position A");
			Vector v_tmp_spoofs;
			for(int i=0;i<(int)sql.m_num_rows;i++){
				if(i%100==0){
					Sleep(50);
				}
				if(i%1000==0){
					log_msg.Format("Added %u out of %u decoys/spoofs...",i,sql.m_num_rows);
					ref.System()->LogQuery(log_msg);
				}
				string proj=sql.mpp_results[i][0].c_str();
				UINT track=atoi(sql.mpp_results[i][1].c_str());
				string file_name=sql.mpp_results[i][2].c_str();
				string title=sql.mpp_results[i][3].c_str();
				string artist=sql.mpp_results[i][4].c_str();
				string album=sql.mpp_results[i][5].c_str();
				string genre=sql.mpp_results[i][6].c_str();
				string comments=sql.mpp_results[i][7].c_str();
				UINT bitrate=atoi(sql.mpp_results[i][8].c_str());
				UINT media_length=atoi(sql.mpp_results[i][9].c_str());
				UINT size=atoi(sql.mpp_results[i][10].c_str());
				UINT popularity=atoi(sql.mpp_results[i][11].c_str());

				while((size%137)!=0)
					size++;

				AresSNSpoof *ns=new AresSNSpoof(proj.c_str(),track,file_name.c_str(),title.c_str(),artist.c_str(),album.c_str(),genre.c_str(),comments.c_str(),bitrate,media_length,size,popularity);
				if(ns->mb_valid)
					v_tmp_spoofs.Add(ns);
				else
					delete ns;
			}

			ref.System()->LogToFile("AresSNSpoofCache::Run() Position B");
			Vector v_tmp_keywords;
			//create a global list of keywords

			//for each spoof...
			for(int i=0;i<(int)v_tmp_spoofs.Size();i++){
				AresSNSpoof *spoof=(AresSNSpoof*)v_tmp_spoofs.Get(i);
				//for each keyword in each spoof...
				for(int j=0;j<(int)spoof->mv_keywords.size();j++){
					//try to add each keyword to our global list of keywords, also add this file onto that keywords file list
					bool b_found=false;
					for(int k=0;k<(int)v_tmp_keywords.Size();k++){
						SNKeyword *kw=(SNKeyword*)v_tmp_keywords.Get(k);
						if(stricmp(spoof->mv_keywords[j].c_str(),kw->m_keyword.c_str())==0){
							b_found=true;
							kw->mv_files.Add(spoof);
							break;
						}
					}
					if(!b_found){
						SNKeyword *nkw=new SNKeyword(spoof->mv_keywords[j].c_str());
						nkw->mv_files.Add(spoof);
						v_tmp_keywords.Add(nkw);
					}
				}
			}

			ref.System()->LogToFile("AresSNSpoofCache::Run() Position C");
			v_tmp_keywords.Sort(1);

			CSingleLock lock(&m_lock,TRUE);
			mv_spoofs.Copy(&v_tmp_spoofs);
			mv_keywords.Copy(&v_tmp_keywords);

			log_msg.Format("Finished loading %u decoys/spoofs from database into the system",mv_spoofs.Size());
			ref.System()->LogQuery(log_msg);
			ref.System()->LogToFile("AresSNSpoofCache::Run() END");
		}
	}
	return 0;
}

bool AresSNSpoofCache::IsReady(void)
{
	if(mv_spoofs.Size()>0)
		return true;
	else return false;
}

void AresSNSpoofCache::GetMatchingSpoofs(int search_type,vector<string>& v_keywords, Vector& v_final_spoofs)
{
	CSingleLock lock(&m_lock,TRUE);


	int keyword_count=0;
	int longest_index=-1;
	string longest_keyword;
	for(int j=0;j<(int)v_keywords.size();j++){
		CString tmp=v_keywords[j].c_str();
		tmp=tmp.Trim();
		if(tmp.GetLength()>0)
			keyword_count++;
		if(tmp.GetLength()>(int)longest_keyword.size()){
			longest_keyword=(LPCSTR)tmp;
			longest_index=j;
		}

		//add a porn filter to reduce workload from all these queries, since every other one is a porn search
		
		/*
		if(stricmp(v_keywords[j].c_str(),"porn")==0
			|| stricmp(v_keywords[j].c_str(),"pussy")==0
			|| stricmp(v_keywords[j].c_str(),"cock")==0
			|| stricmp(v_keywords[j].c_str(),"dick")==0
			|| stricmp(v_keywords[j].c_str(),"cunt")==0
			|| stricmp(v_keywords[j].c_str(),"ass")==0
			|| stricmp(v_keywords[j].c_str(),"facial")==0
			|| stricmp(v_keywords[j].c_str(),"cum")==0
			|| stricmp(v_keywords[j].c_str(),"fuck")==0
			|| stricmp(v_keywords[j].c_str(),"fucking")==0
			|| stricmp(v_keywords[j].c_str(),"yr")==0
			|| stricmp(v_keywords[j].c_str(),"rape")==0
			//|| stricmp(v_keywords[j].c_str(),"sex")==0  //there are probably too many real songs with this in the title
			|| stricmp(v_keywords[j].c_str(),"gangbang")==0
			|| stricmp(v_keywords[j].c_str(),"butt")==0
			|| stricmp(v_keywords[j].c_str(),"latina")==0
			|| stricmp(v_keywords[j].c_str(),"skat")==0
			|| stricmp(v_keywords[j].c_str(),"scat")==0
			|| stricmp(v_keywords[j].c_str(),"boobs")==0){
				return;
			}*/
	}

	if(keyword_count<1 || longest_keyword.size()<4)  //no valid keywords in this query
		return;

	if(longest_index!=-1){
		v_keywords.erase(v_keywords.begin()+longest_index);  //no need to test every file for this keyword, so lets pull it out of the vector
	}


	if(keyword_count==1){  //did they just type in a number for a search string?
		bool b_all_numeric=true;
		for(int i=0;i<(int)longest_keyword.size();i++){
			byte b=longest_keyword[i];
			if(!(b<128 && isdigit(b))){  //have to do b<128 test or those functions crash for some reason
				b_all_numeric=false;
				break;
			}
		}

		if(b_all_numeric){
			//someone just typed in a search string like '2005', we should probably ignore this
			return;
		}
	}

	/*
	if(longest_keyword.size()==3){
		if(stricmp(longest_keyword.c_str(),"the")==0 
			|| stricmp(longest_keyword.c_str(),"its")==0
			|| stricmp(longest_keyword.c_str(),"was")==0
			|| stricmp(longest_keyword.c_str(),"who")==0
			|| stricmp(longest_keyword.c_str(),"two")==0
			|| stricmp(longest_keyword.c_str(),"fun")==0){
				return;
			}
	}*/

	Vector v_tmp_spoofs;

	//quickly get all the files that contain the longest keyword
	SNKeyword kw(longest_keyword.c_str());  //only need to look up one keyword
	int index=mv_keywords.BinaryFind(&kw);
	if(index!=-1){
		SNKeyword *kw=(SNKeyword*)mv_keywords.Get(index);
		v_tmp_spoofs.Copy(&kw->mv_files);
	}


	//test each of those files for the rest of the keywords, note that we removed longest_keyword from that list for efficiency, since they obviously have it
	for(int i=0;i<(int)v_tmp_spoofs.Size();i++){
		AresSNSpoof* s=(AresSNSpoof*)v_tmp_spoofs.Get(i);

		//only working with audio and video types for now
		if(!s->IsSearchType(search_type))
			continue;
		bool b_valid=true;
		for(int j=0;j<(int)v_keywords.size();j++){
			if(!s->HasKeyword(v_keywords[j].c_str())){
				b_valid=false;
				break;
			}
		}
		if(b_valid){
			v_final_spoofs.Add(s);
		}
	}
}

UINT AresSNSpoofCache::GetLoadedDecoys(void)
{
	return mv_spoofs.Size();;
}
