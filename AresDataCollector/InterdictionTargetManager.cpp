#include "StdAfx.h"
#include "interdictiontargetmanager.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\TinySQL.h"
#include "AresDataCollectorSystem.h"

InterdictionTargetManager::InterdictionTargetManager(void)
{
	m_target_index=0;
}

InterdictionTargetManager::~InterdictionTargetManager(void)
{
}

UINT InterdictionTargetManager::Run(void)
{
	AresDataCollectorSystemReference ref;
	ref.System()->Log("Ares System: Interdiction Target Manager Thread Started.");
	bool b_loaded=false;

	Timer last_load;

	while(!this->b_killThread){
		Sleep(300);

		if(last_load.HasTimedOut(30*60) || !b_loaded){
			last_load.Refresh();
			b_loaded=true;
			//vector<unsigned int> v_ips;
			//vector<unsigned short> v_ports;		
			//vector<string> v_hashes;		
			//vector<unsigned int> v_sizes;		
			TinySQL sql;
			ref.System()->Log("Ares System: Interdiction Target Manager Loading Interdiction Targets...");

			
			if(!sql.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306))
				continue;
			if(!sql.Query("select count(*) from ares_interdiction",true))
				continue;

			int count=atoi(sql.mpp_results[0][0].c_str());

			int index=0;

			vector <string> v_exception_hashes;
			ref.System()->GetProjectCache()->GetExceptionHashes(v_exception_hashes);

			Vector v_tmp;
			while(index<count){
				if(this->b_killThread)
					return 0;

				CString query;
				query.Format("select ip,port,hash,size from ares_interdiction limit %d,50000",index);

				if(!sql.Query(query,true))
					continue;

				index+=50000;
				
				
				for(int i=0;i<(int)sql.m_num_rows && !this->b_killThread;i++){
					const char* ip=sql.mpp_results[i][0].c_str();
					
					if(strlen(sql.mpp_results[i][2].c_str())!=40)  //make sure hash is proper length or instability will follow
						continue;

					bool b_add=true;
					for(int exception_index=0;exception_index<(int)v_exception_hashes.size();exception_index++){
						if(stricmp(sql.mpp_results[i][2].c_str(),v_exception_hashes[exception_index].c_str())==0){
							b_add=false;
							break;
						}
					}

					if(b_add){
						AresInterdictionTarget *nit=new AresInterdictionTarget((unsigned int)inet_addr(sql.mpp_results[i][0].c_str()),(unsigned short)atoi(sql.mpp_results[i][1].c_str()),(unsigned int)atoi(sql.mpp_results[i][3].c_str()),sql.mpp_results[i][2].c_str());
						v_tmp.Add(nit);
					}
				}
				Sleep(100);
			}

			if(this->b_killThread)
				return 0;

			{
				CSingleLock lock(&m_lock,TRUE);
				mv_targets.Copy(&v_tmp);
			}
			ref.System()->Log("Ares System: Interdiction Target Manager finished loading interdiction targets.");
			ref.System()->Log("Ares System: Interdiction Target Manager cleaning interdiction table....");
			sql.Query("delete from ares_interdiction where time_created=(now()-INTERVAL 2 DAY)",false);
			sql.Query("delete from ares_interdiction where time_created=(now()-INTERVAL 10 DAY)",false);
			ref.System()->Log("Ares System: Interdiction Target Manager finished cleaning interdiction table.");
			last_load.Refresh();
		}


	}
	return 0;
}

void InterdictionTargetManager::GetSomeTargets(Vector &v_tmp,int count)
{
	CSingleLock lock(&m_lock,TRUE);
	for(int i=0;i<count;i++){
		if(m_target_index>=(int)mv_targets.Size())
			m_target_index=0;
		if(m_target_index>=(int)mv_targets.Size())
			return;

		v_tmp.Add(mv_targets.Get(m_target_index++));
	}
}

int InterdictionTargetManager::GetInterdictionTargetCount(void)
{
	CSingleLock lock(&m_lock,TRUE);
	return (int)mv_targets.Size();
}
