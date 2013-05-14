#include "StdAfx.h"
#include "aresdecoypool.h"
#include "AresPoolFile.h"
#include "..\tkcom\tinysql.h"
#include "..\tkcom\timer.h"

#define BIGFILETHREASHOLD 20000000

AresDecoyPool::AresDecoyPool(void)
{
	mb_ready=false;

	m_small_index1=0;
	m_small_index2=0;

	m_big_index1=0;
	m_big_index2=0;
}

AresDecoyPool::~AresDecoyPool(void)
{
}

UINT AresDecoyPool::Run(void)
{
	Timer reload_timer;
	bool b_has_loaded=false;

	int init_delay=10+(rand()%LOADDELAY);
	Timer startup;

	while(!startup.HasTimedOut(init_delay) & !this->b_killThread){
		Sleep(1000);
	}

	while(!this->b_killThread){
		Sleep(100);
		
		if(reload_timer.HasTimedOut(60*60) || !b_has_loaded || (!mb_ready && reload_timer.HasTimedOut(20*60))){
			srand(GetTickCount());

			bool b_update_database=false;
			if((rand()%7)==0)
				b_update_database=true;

			reload_timer.Refresh();
			b_has_loaded=true;
			TinySQL sql;
			if(!sql.Init("206.161.141.35","onsystems","ebertsux37","ares_data",3306))
				continue;
			if(!sql.Query("select now()",true))
				continue;

			//lets query a seed based on current month and day
			int seed=15;
			if(sql.m_num_rows>0){
				CString the_date=sql.mpp_results[0][0].c_str();
				CString month=the_date.Mid(5,2);
				CString day=the_date.Mid(8,2);
				TRACE("AresDecoyPool::Run() Creating decoy pool: seed month was %s, seed day was %s\n",month,day);
				seed=(int)(((atoi(month)*32)+(atoi(day)))/3)+1;
			}

			
			//byte sig[SIGNATURELENGTH];
//			DonkeyFile tmp("",50000,sig,false,false,true);
			srand(seed);

			vector <UINT> v_music_sizes;
			vector <UINT> v_movie_sizes;

			int num_files=1500;

			for(int i=0;i<num_files;i++){
				
				int tmp_music_size=3000000+(3000*(rand()%1000))+rand();  //between 3 and 5 megs
				int tmp_movie_size=500000000+((rand()%400)*(rand()%1000)*(rand()%1000))+rand();  //between 500 and 900 megs
				while((tmp_music_size%137)!=0)
					tmp_music_size++;

				while((tmp_movie_size%137)!=0)
					tmp_movie_size++;

				v_music_sizes.push_back(tmp_music_size);
				v_movie_sizes.push_back(tmp_movie_size);
			}

			Vector v_tmp_small_files[256];
			Vector v_tmp_big_files[256];

			//create X music decoys
			for(int i=0;i<num_files && !this->b_killThread;i++){
				Sleep(5);
				if((i%25)==0){
					TRACE("Creating new music decoy %u out of %u for decoy pool...\n",i,num_files);
				}
				AresPoolFile *pf=new AresPoolFile();
				pf->Generate(v_music_sizes[i]);

				for(int j=0;j<(int)v_tmp_small_files[pf->m_hash[0]].Size();j++){
					AresPoolFile *af=(AresPoolFile*)v_tmp_small_files[pf->m_hash[0]].Get(j);
					if(memcmp(pf->m_hash,af->m_hash,20)==0){
						delete pf;  //this hash has already been generated, we should never get here but just in case
						pf=NULL;
						break;
					}
				}


				v_tmp_small_files[pf->m_hash[0]].Add(pf);
			}

			//create X movie decoys
			for(int i=0;i<(num_files>>2) && !this->b_killThread;i++){
				Sleep(5);
				if((i%25)==0){
					TRACE("Created new movie decoy %u out of %u for decoy pool...\n",i,num_files);
				}
				AresPoolFile *pf=new AresPoolFile();
				pf->Generate(v_movie_sizes[i]);

				for(int j=0;j<(int)v_tmp_big_files[pf->m_hash[0]].Size();j++){
					AresPoolFile *af=(AresPoolFile*)v_tmp_big_files[pf->m_hash[0]].Get(j);
					if(memcmp(pf->m_hash,af->m_hash,20)==0){
						delete pf;  //this hash has already been generated, we should never get here but just in case
						pf=NULL;
						break;
					}
				}

				if(pf!=NULL)
					v_tmp_big_files[pf->m_hash[0]].Add(pf);
			}

			CSingleLock lock(&m_lock,FALSE);
			lock.Lock();
			for(int i=0;i<256;i++){
				mv_small_decoys[i].Copy(&v_tmp_small_files[i]);
			}
			for(int i=0;i<256;i++){
				mv_big_decoys[i].Copy(&v_tmp_big_files[i]);
			}
			lock.Unlock();
			TRACE("Finished creating decoys\n");
			//report these decoys to the database

			if(!this->b_killThread && b_update_database){
				TinySQL sql2;
				if(!sql2.Init("206.161.141.35","onsystems","ebertsux37","ares_data",3306))
					continue;

				CString query="insert ignore into ares_decoy_pool values ";
				int insert_count=0;

				for(int i=0;i<256;i++){
					for(int j=0;j<(int)mv_small_decoys[i].Size();j++){
						AresPoolFile *pf=(AresPoolFile*)mv_small_decoys[i].Get(j);

						CString tmp;
						if(insert_count++>0)
							tmp.Format(",('%s',%u,now())",pf->m_shash.c_str(),pf->m_size);
						else
							tmp.Format("('%s',%u,now())",pf->m_shash.c_str(),pf->m_size);
						query+=tmp;
					}
					for(int j=0;j<(int)mv_big_decoys[i].Size();j++){
						AresPoolFile *pf=(AresPoolFile*)mv_big_decoys[i].Get(j);

						CString tmp;
						if(insert_count++>0)
							tmp.Format(",('%s',%u,now())",pf->m_shash.c_str(),pf->m_size);
						else
							tmp.Format("('%s',%u,now())",pf->m_shash.c_str(),pf->m_size);
						query+=tmp;
					}
				}
				if(insert_count>0 && !this->b_killThread){
					sql2.Query("delete from ares_decoy_pool",false);
					sql2.Query(query,false);
				}
			}
			mb_ready=true;
		}

		
	}
	return 0;
}

bool AresDecoyPool::GetNextDecoy(UINT desired_size, Vector& v)
{
	if(desired_size<BIGFILETHREASHOLD){
		int attempt=0;
		while(v.Size()==0 && attempt++<(256/4)){
			if(m_small_index1>=256)
				m_small_index1=0;

			if(m_small_index2>=(int)mv_small_decoys[m_small_index1].Size()){
				m_small_index2=0;
				m_small_index1++;  //advance our bin index by 1
				continue;
			}

			if(m_small_index2>=(int)mv_small_decoys[m_small_index1].Size()){
				continue;
			}
			else{
				v.Add(mv_small_decoys[m_small_index1].Get(m_small_index2++));
				return true;
			}
		}		
	}
	else{
		int attempt=0;
		while(v.Size()==0 && attempt++<(256/4)){
			if(m_big_index1>=256)
				m_big_index1=0;

			if(m_big_index2>=(int)mv_big_decoys[m_big_index1].Size()){
				m_big_index2=0;
				m_big_index1++;  //advance our bin index by 1
				continue;
			}

			if(m_big_index2>=(int)mv_big_decoys[m_big_index1].Size()){
				continue;
			}
			else{
				v.Add(mv_big_decoys[m_big_index1].Get(m_big_index2++));
				return true;
			}
		}		
	}
	return false;
}

bool AresDecoyPool::GetDecoy(byte* hash, Vector& v)
{
	CSingleLock lock(&m_lock,TRUE);

	//try looking in our smaller files
	for(int i=0;i<(int)mv_small_decoys[hash[0]].Size();i++){
		AresPoolFile* apf=(AresPoolFile*)mv_small_decoys[hash[0]].Get(i);
		if(memcmp(apf->m_hash,hash,20)==0){
			v.Add(apf);
			return true;
		}
	}


	//try looking in our bigger files
	for(int i=0;i<(int)mv_big_decoys[hash[0]].Size();i++){
		AresPoolFile* apf=(AresPoolFile*)mv_big_decoys[hash[0]].Get(i);
		if(memcmp(apf->m_hash,hash,20)==0){
			v.Add(apf);
			return true;
		}
	}

	return false;
}
