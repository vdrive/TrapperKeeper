#include "StdAfx.h"
#include "aressnhostcache.h"
#include "..\tkcom\timer.h"
#include "..\tkcom\TinySQL.h"
#include "AresSupernodeSystem.h"
#include "AresUserName.h"

AresSNHostCache::AresSNHostCache(void)
{
	m_index1=0;
	m_index2=0;

	m_user_index=0;
	m_num_hosts=0;
}

AresSNHostCache::~AresSNHostCache(void)
{
}

UINT AresSNHostCache::Run(void)
{
	Timer last_reload;
	bool b_has_loaded=false;
	srand(GetTickCount());
	int delay=10+(rand()%LOADDELAY);  //TYDEBUG

	Timer last_sn_request;

	Timer last_host_save;

	Timer last_user_name_load;
	bool b_has_loaded_user_names=false;

	UINT save_attempt=0;

	while(!this->b_killThread){
		Sleep(100);

		if((!b_has_loaded_user_names && last_user_name_load.HasTimedOut(delay)) || last_user_name_load.HasTimedOut(60*60*12) || (mv_user_names.Size()==0 && last_user_name_load.HasTimedOut(60*60*2))){
			TRACE("AresSNHostCache::Run(void) QUERY USER NAMES BEGIN\n");
			AresSupernodeSystemRef ref;
			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY USER NAMES BEGIN");
			ref.System()->LogQuery("Loading user names...");
			b_has_loaded_user_names=true;
			TinySQL sql;
			last_user_name_load.Refresh();
			if(this->b_killThread || !sql.Init("63.216.246.62","onsystems","tacobell","metamachine",3306)){
				ref.System()->LogToFile("AresSNHostCache::Run() END b");
				TRACE("AresSNHostCache::Run(void) QUERY USER NAMES ABORTED, FAILED QUERY\n");
				continue;
			}
			
			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY USER NAMES POSITION A");
			if(!sql.Query("select name from user_names order by rand() limit 8000",true)){
				ref.System()->LogToFile("AresSNHostCache::Run() END c");
				TRACE("AresSNHostCache::Run(void) QUERY USER NAMES ABORTED, FAILED QUERY\n");
				continue;
			}

			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY USER NAMES POSITION B");
			Vector v_tmp_names;
			for(int i=0;i<(int)sql.m_num_rows;i++){
				CString name=sql.mpp_results[i][0].c_str();
				name+="@Ares";
				CString original_name=name;
				CString l_name=name.MakeLower();

				//get rid of the ones that actually reference the network that they came from
				if(l_name.Find("mule")!=-1 || l_name.Find("donkey")!=-1 || l_name.Find("gaie")!=-1 || l_name.Find("shar")!=-1 || l_name.Find("razor")!=-1){
					continue;
				}
				v_tmp_names.Add(new AresUserName(original_name));
			}

			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY USER NAMES POSITION C");

			CSingleLock lock(&m_lock,TRUE);
			mv_user_names.Copy(&v_tmp_names);	

			CString log_msg;
			log_msg.Format("Loaded %u user names from the database",mv_user_names.Size());

			ref.System()->LogQuery(log_msg);

			TRACE("AresSNHostCache::Run(void) QUERY USER NAMES END\n");
			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY USER NAMES END");
		}

		if(last_sn_request.HasTimedOut(10)){
			last_sn_request.Refresh();
			m_interface.RequestSupernodes();
		}

		if(last_host_save.HasTimedOut(15)){
			last_host_save.Refresh();
			while(m_interface.mv_saved_hosts.Size()>0){
				SaveHostObject *sh=(SaveHostObject*)m_interface.mv_saved_hosts.Get(0);

				int index=GetBinFromPeer(sh->m_ip.c_str());
				bool b_found=false;
				
				for(int i=0;i<(int)mv_hosts_array[index].Size();i++){
					AresHost* ah=(AresHost*)mv_hosts_array[index].Get(i);
					if(stricmp(ah->GetIP(),sh->m_ip.c_str())==0){
						b_found=true;
						break;
					}
				}
				if(!b_found){
					if(sh->m_port!=4685 && sh->m_port!=BASEPORT){
						mv_hosts_array[index].Add(new AresHost(sh->m_ip.c_str(),sh->m_port));
						m_num_hosts++;
					}
				}

				m_interface.mv_saved_hosts.Remove(0);
			}

			if(mv_hosts_to_save.Size()>30){
				vector<string> v_ips;
				vector<unsigned short> v_ports;
				int saved_count=0;
				while(mv_hosts_to_save.Size()>0 && saved_count++<45){
					SaveHostObject *sh=(SaveHostObject*)mv_hosts_to_save.Get(0);
					v_ips.push_back(sh->m_ip.c_str());
					v_ports.push_back(sh->m_port);
					mv_hosts_to_save.Remove(0);
				}	
				m_interface.ReportNewSupernodesToServer(v_ips,v_ports);
			}
		}

/*
		if(last_host_save.HasTimedOut(60)){  //every X minutes
			save_attempt++;
			AresSupernodeSystemRef ref;
			ref.System()->LogToFile("AresSNHostCache::Run(void) Save Host BEGIN");

			last_host_save.Refresh();

			if(mv_hosts_to_save.Size()==0){
				ref.System()->LogToFile("AresSNHostCache::Run(void) Save Host END 0");
				continue;
			}

			ref.System()->LogToFile("AresSNHostCache::Run(void) Save Host POSITION A");


			CString query="insert ignore into ares_hosts values ";

			ref.System()->LogToFile("AresSNHostCache::Run(void) Save Host POSITION B");
			
			CString tmp;
			int count=0;
			while(mv_hosts_to_save.Size()>0 && count<200){
				SaveHostObject *sh=(SaveHostObject*)mv_hosts_to_save.Get(0);
				if(count>0){
					tmp.Format(",('%s',%u,now())",sh->m_ip.c_str(),sh->m_port);
				}
				else{
					tmp.Format("('%s',%u,now())",sh->m_ip.c_str(),sh->m_port);
				}

				if(mv_hosts.Size()<50000){
					bool b_found=false;
					for(int i=0;i<(int)mv_hosts.Size();i++){
						AresHost *ah=(AresHost*)mv_hosts.Get(i);
						if(ah->GetPort()==sh->m_port && stricmp(ah->GetIP(),sh->m_ip.c_str())==0){
							b_found=true;
							break;
						}
					}

					if(!b_found)
						mv_hosts.Add(new AresHost(sh->m_ip.c_str(),sh->m_port));  //add this host to our master list so we can connect to it soon
				}
				query+=tmp;

				mv_hosts_to_save.Remove(0);
				count++;
			}

			ref.System()->LogToFile("AresSNHostCache::Run(void) Save Host POSITION C");

			if((save_attempt%120)==0){  //every X of these we'll save some hosts to the database to keep it current
				TinySQL sql;
				if(this->b_killThread || !sql.Init("38.119.64.101","onsystems","ebertsux37","ares_data",3306)){
					ref.System()->LogToFile("AresSNHostCache::Run(void) Save Host END 1");
					continue;
				}

				if(!sql.Query(query,false)){
					TRACE("AresSNHostCache::Run(void) FAILED HOST SAVE REASON='%s'\n",sql.m_fail_reason.c_str());
					ASSERT(0);
				}
			}

			last_host_save.Refresh();
			ref.System()->LogToFile("AresSNHostCache::Run(void) Save Host END");
		}
		if((!b_has_loaded && last_reload.HasTimedOut(delay)) || (last_reload.HasTimedOut(60*60*6)) || (mv_hosts.Size()==0 && last_reload.HasTimedOut(60*60*2))){
			CString log_msg;
			b_has_loaded=true;
			TRACE("AresSNHostCache::Run(void) QUERY HOSTS BEGIN\n");
			AresSupernodeSystemRef ref;
			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS BEGIN");
			ref.System()->LogQuery("Loading hosts...");
			last_reload.Refresh();


			//TYDEBUG

			//mv_hosts.Add(new AresHost("65.28.246.249",58701));
			//mv_hosts.Add(new AresHost("70.49.38.18",10101));
			//mv_hosts.Add(new AresHost("67.166.253.165",63895));

//Ares.exe:2916	TCP	81.3.87.174:1942	65.28.246.249:58701	ESTABLISHED	
//Ares.exe:2916	TCP	81.3.87.174:2918	70.49.38.18:10101	ESTABLISHED	
//Ares.exe:2916	TCP	81.3.87.174:2806	67.166.253.165:63895	ESTABLISHED	


			int offset=0;
			
			
			TinySQL sql;
			if(this->b_killThread || !sql.Init("38.119.64.101","onsystems","ebertsux37","ares_data",3306)){
				ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS END 0");
				TRACE("AresSNHostCache::Run(void) QUERY HOSTS END, QUERY FAIL\n");
				continue;
			}

			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS POSITION A");


////IMPORT
			if(!sql.Query("select count(*) from ares_hosts",true)){  //TYDEBUG
				log_msg.Format("FAILED to query database for hosts because %s. (will retry later) ",sql.m_fail_reason.c_str());
				ref.System()->LogQuery(log_msg);
				ref.System()->LogToFile("AresSNHostCache::Run() END 1");
				continue;
			}

			if(sql.m_num_rows<1){
				log_msg.Format("FAILED to query database for hosts because Corrupt Count. (will retry later) ");
				ref.System()->LogQuery(log_msg);
				ref.System()->LogToFile("AresSNHostCache::Run() END 1");
				continue;
			}

			int count=atoi(sql.mpp_results[0][0].c_str());


			offset=rand()%(max(1,max(1,count-10000)/100));
			offset*=100;  //offset was scaled down by 100 because rand() only goes up to 65k
		
			offset=max(0,offset-10000); //we do this so that we have a good chance of selecting the ones in front and the back, since we are randoming for a range.  If we didn't do this there would be about 1/150000 chance that the start would be picked, not good
			offset=min(offset,max(0,count-30000));

			CString query;
			query.Format("SELECT ip,port from ares_hosts limit %u,30000",offset);

			if(!sql.Query(query,true)){  //TYDEBUG
				log_msg.Format("FAILED to query database for hosts because %s. (will retry later) ",sql.m_fail_reason.c_str());
				ref.System()->LogQuery(log_msg);
				ref.System()->LogToFile("AresSNHostCache::Run() END 1");
				continue;
			}
			
////END IMPORT


//			if(this->b_killThread || !sql.Query("SELECT ip,port from ares_hosts order by rand() limit 15000",true)){
//				TRACE("AresSNHostCache::Run(void) QUERY HOSTS END, QUERY FAIL\n");
//				ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS END 1");
//				continue;
//			}

			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS POSITION B");
			Vector v_tmp;

			Vector v_tmp2;

			for(int i=0;i<(int)sql.m_num_rows;i++){
				AresHost *as=new AresHost(sql.mpp_results[i][0].c_str(),atoi(sql.mpp_results[i][1].c_str()));
				if(as->GetPort()==4685)
					v_tmp2.Add(as);
				else
					v_tmp.Add(as);
			}

			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS POSITION C");
			CSingleLock lock(&m_lock,TRUE);
			CSingleLock lock2(&m_md_host_lock,TRUE);
			mv_hosts.Copy(&v_tmp);
			mv_md_hosts.Copy(&v_tmp2);
			
			
			m_index=rand()%max(1,mv_hosts.Size());  //set this one to be a random offset

			ref.System()->LogToFile("AresSNHostCache::Run() Position B");
			log_msg.Format("Loaded %u supernodes from the database with start_offset=%u",mv_hosts.Size(),offset);
			
			ref.System()->LogQuery(log_msg);

			b_has_loaded=true;

			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS END");
			TRACE("AresSNHostCache::Run(void) QUERY HOSTS END\n");
		}
		*/
	}
	return 0;
}

bool AresSNHostCache::GetNextHost(Vector &v,bool tcp_ip)
{

	if(tcp_ip && mv_reconnect_hosts.Size()>0){
		//we have a host we'd like to try to reconnect to.  This was probably a host we've been connected to but lost the connection for whatever reason.
		AresHost *ah=(AresHost*)mv_reconnect_hosts.Get(0);
		v.Add(ah);
		mv_reconnect_hosts.Remove(0);
		return true;
	}


	int attempt=0;
	while(v.Size()==0 && attempt++<(NUMCONBINS/4)){
		if(m_index1>=NUMCONBINS)
			m_index1=0;

		if(m_index2>=mv_hosts_array[m_index1].Size()){
			m_index2=0;
			m_index1++;  //advance our bin index by 1
			continue;
		}

		if(m_index2>=mv_hosts_array[m_index1].Size()){
			continue;
		}
		else{
			v.Add(mv_hosts_array[m_index1].Get(m_index2++));
		}
	}

/*
	CSingleLock lock(&m_lock,TRUE);
	if(m_index>=mv_hosts.Size())
		m_index=0;
	if(m_index>=mv_hosts.Size())
		return false;
	
	v.Add(mv_hosts.Get(m_index++));
*/
	//ip=sn->m_ip;
	//port=sn->m_port;
	return true;
}

UINT AresSNHostCache::GetKnownHosts(void)
{
	return m_num_hosts;
}

bool AresSNHostCache::IsReady(void)
{
	return (m_num_hosts>0)?true:false;
}

void AresSNHostCache::SaveHost(const char* host_ip, unsigned short port)
{
	if(mv_hosts_to_save.Size()<500){
		
		//see if we already know about this host
		int index=GetBinFromPeer(host_ip);

		bool b_found=false;
		
		for(int i=0;i<(int)mv_hosts_array[index].Size();i++){
			AresHost* ah=(AresHost*)mv_hosts_array[index].Get(i);
			if(stricmp(ah->GetIP(),host_ip)==0){
				b_found=true;
				break;
			}
		}

		if(!b_found){
			//to cut down on traffic only save this if this client doesn't know about it.
			//if this client does know about it, then that means the supernode server knows about it because thats where we got it.
			mv_hosts_to_save.Add(new SaveHostObject(host_ip,port));
		}
	}
}

void AresSNHostCache::GetSomeUserNames(int amount, Vector& v_tmp)
{
	CSingleLock lock(&m_lock,TRUE);
	if((int)mv_user_names.Size()<=amount){
		//not enough user names, create anons
		for(int i=0;i<amount;i++){
			CString anon;
			//sprintf(m_user_name,"blaha%x%x%x%03x",fd,sd,td,rand() % 0xFFF);
			anon.Format("anon_%x%x%x%x%x%x%x%x@Ares",rand()%16,rand()%16,rand()%16,rand()%16
											   ,rand()%16,rand()%16,rand()%16,rand()%16);
			v_tmp.Add(new AresUserName(anon));
		}
	}
	else{
		for(int i=0;i<amount;i++){
			if((rand()%5)==0){  //randomly insert an anon_xxxxxx@Ares name
				CString anon;
				//sprintf(m_user_name,"blaha%x%x%x%03x",fd,sd,td,rand() % 0xFFF);
				anon.Format("anon_%x%x%x%x%x%x%x%x@Ares",rand()%16,rand()%16,rand()%16,rand()%16
												,rand()%16,rand()%16,rand()%16,rand()%16);
				v_tmp.Add(new AresUserName(anon));		
			}
			else{
				if(m_user_index>=mv_user_names.Size())
					m_user_index=0;
				
				v_tmp.Add(mv_user_names.Get(m_user_index++));			
			}
		}
	}
}

UINT AresSNHostCache::GetLoadedUserNames(void)
{
	return mv_user_names.Size();;
}

bool AresSNHostCache::GetRandomMDHost(Vector &v_tmp)
{
	CSingleLock lock(&m_md_host_lock,TRUE);
	if(mv_md_hosts.Size()==0)
		return false;

	int rand_index=rand()%mv_md_hosts.Size();
	AresHost* h=(AresHost*)mv_md_hosts.Get(rand_index);
	v_tmp.Add(h);
	return true;
}

void AresSNHostCache::StartSystem(void)
{
	m_interface.StartSystem();
	this->StartThreadLowPriority();
}

void AresSNHostCache::StopSystem(void)
{
	this->StopThread();
	m_interface.StopSystem();
}

void AresSNHostCache::SNInterface::ReceivedSupernodes(vector<string> &v_ips, vector<unsigned short> &v_ports){
	for(int i=0;i<(int)v_ips.size();i++){
		mv_saved_hosts.Add(new SaveHostObject(v_ips[i].c_str(),v_ports[i]));
	}
}

//call this to prioritize reconnecting to a host.  This should probably be done if we lost a connection to a good host.
void AresSNHostCache::AttemptReconnection(const char* ip, unsigned short port)
{
	mv_reconnect_hosts.Add(new AresHost(ip,port));
}
