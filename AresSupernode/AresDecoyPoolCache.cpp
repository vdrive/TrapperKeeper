#include "StdAfx.h"
#include "aresdecoypoolcache.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\TinySQL.h"
#include "AresSNDecoy.h"

AresDecoyPoolCache::AresDecoyPoolCache(void)
{
	m_movie_index1=0;
	m_movie_index2=0;

	m_music_index1=0;
	m_music_index2=0;

	m_num_music_decoys=0;
	m_num_movie_decoys=0;
}

AresDecoyPoolCache::~AresDecoyPoolCache(void)
{
}

UINT AresDecoyPoolCache::Run(void)
{
	Timer last_update;
	bool b_has_updated=false;

	int delay=10+(rand()%LOADDELAY);  //TYDEBUG
	for(int i=0;i<delay && !this->b_killThread;i++){ //sleep for X seconds before we start the processor (in case the assembly.bin file hasn't quite finished synching yet or its causing it to crash for some reason, at least we'll be able to patch before TK crashes)
		Sleep(1000);  
	}

	while(!this->b_killThread){
		Sleep(100);
		if(last_update.HasTimedOut(4*60*60) || !b_has_updated){
			last_update.Refresh();
			b_has_updated=true;

			TinySQL sql;
			if(!sql.Init("206.161.141.35","onsystems","ebertsux37","ares_data",3306))
				continue;

			if(!sql.Query("select hash,size from ares_decoy_pool",true))
				continue;
			CSingleLock lock(&m_lock,FALSE);
			lock.Lock();

			for(int i=0;i<NUMDECOYBINS;i++){
				mv_music_decoys[i].Clear();
				mv_movie_decoys[i].Clear();
			}
			m_num_music_decoys=0;
			m_num_movie_decoys=0;

			for(int i=0;i<(int)sql.m_num_rows;i++){
				int size=atoi(sql.mpp_results[i][1].c_str());
				const char *hash=sql.mpp_results[i][0].c_str();
				if(strlen(hash)<40)
					continue;
				AresSNDecoy *asnd=new AresSNDecoy(hash,(UINT)size);
				if(size<10000000){
					m_num_music_decoys++;
					mv_music_decoys[asnd->m_size%NUMDECOYBINS].Add(asnd);
				}
				else{
					m_num_movie_decoys++;
					mv_movie_decoys[asnd->m_size%NUMDECOYBINS].Add(asnd);
				}
			}

			lock.Unlock();
		}
	}
	return 0;
}

bool AresDecoyPoolCache::GetDecoy(UINT& actual_size, byte* actual_hash, byte* desired_hash,UINT desired_size)
{
	CSingleLock lock(&m_lock,TRUE);
	if(desired_size<10000000){
		int index=desired_size%NUMDECOYBINS;
		int attempt=0;
		while(attempt++<(NUMDECOYBINS>>1) && mv_music_decoys[index].Size()<1){
			index++;
			if(index>=NUMDECOYBINS)
				index=0;
		}

		if(mv_music_decoys[index].Size()==0)
			return false;


		UINT min_dif=1000000000;
		AresSNDecoy *target=NULL;
		for(int i=0;i<(int)mv_music_decoys[index].Size();i++){
			AresSNDecoy *decoy=(AresSNDecoy *)mv_music_decoys[index].Get(i);
			UINT dif=decoy->CalcDiff(desired_hash);
			if(dif<min_dif){
				target=decoy;
				min_dif=dif;
			}
		}	

		if(target!=NULL){
			actual_size=target->m_size;
			memcpy(actual_hash,target->m_hash,20);
			return true;
		}
	}
	else{
		int index=desired_size%NUMDECOYBINS;
		int attempt=0;
		while(attempt++<(NUMDECOYBINS>>1) && mv_movie_decoys[index].Size()<1){
			index++;
			if(index>=NUMDECOYBINS)
				index=0;
		}

		if(mv_movie_decoys[index].Size()==0)
			return false;


		UINT min_dif=1000000000;
		AresSNDecoy *target=NULL;
		for(int i=0;i<(int)mv_movie_decoys[index].Size();i++){
			AresSNDecoy *decoy=(AresSNDecoy *)mv_movie_decoys[index].Get(i);
			UINT dif=decoy->CalcDiff(desired_hash);
			if(dif<min_dif){
				target=decoy;
				min_dif=dif;
			}
		}	

		if(target!=NULL){
			actual_size=target->m_size;
			memcpy(actual_hash,target->m_hash,20);
			return true;
		}
	}
	return false;
}
