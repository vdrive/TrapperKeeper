#include "StdAfx.h"
#include "areshostcache.h"
#include ".\areshostcache.h"
#include <mmsystem.h>
#include "..\tkcom\TinySQL.h"

AresHostCache::AresHostCache(void)
{
	mb_has_saved_once=false;
	m_host_offset=0;
	mb_ready=false;
}

AresHostCache::~AresHostCache(void)
{
}

/*
void AresHostCache::ReadInHostFile(void)
{
	CSingleLock lock(&m_lock,TRUE);
	CreateDirectory("c:\\AresProtector",NULL);

	HANDLE file = CreateFile("c:\\AresProtector\\hosts.dat",			// open file at local_path 
                GENERIC_READ,              // open for writing 
                0,							// we don't like to share
                NULL,						// no security 
                OPEN_EXISTING,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	

	if(file==INVALID_HANDLE_VALUE || file==NULL){
		//file doesn't exist, so lets open the default one that got synched
		file = CreateFile("c:\\syncher\\rcv\\executables\\areshosts.dat",			// open file at local_path 
					GENERIC_READ,              // open for writing 
					0,							// we don't like to share
					NULL,						// no security 
					OPEN_EXISTING,				// create new whether file exists or not
					FILE_ATTRIBUTE_NORMAL,		// normal file 
					NULL);						// no attr. template 
		if(file==INVALID_HANDLE_VALUE || file==NULL){
			CString log_msg;
			int error=GetLastError();
			//log_msg.Format("FAILED:  Couldn't create the file %s to received it from %s, system error code = %d.\n",m_local_path.c_str(),m_source.c_str(),GetLastError());
			//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
			TRACE("AresHostCache::ReadInHostFile(void) failed because it couldn't open the file.  Error code = %d\n",error);
			return;
		}
	}
	
	DWORD bleh=0;
	DWORD file_length=GetFileSize(file,&bleh);
	
	if(file_length==0 || file_length>1000000){
		CloseHandle(file);
		return;
	}
	
	mv_hosts.Clear();

	byte *data=new byte[file_length];

	ReadFile(file,data,file_length,&bleh,NULL);
	byte *ptr=data;
	byte *end=data+file_length;
	while(ptr<end){
		mv_hosts.Add(new AresHost(*(UINT*)ptr,*(unsigned short*)(ptr+4)));
		ptr+=6;
	}

	mv_hosts.Sort(1);
	
	TRACE("AresHostCache::ReadInHostFile(void) added %d hosts.\n",mv_hosts.Size()); 

	delete [] data;
	CloseHandle(file);
	if(mv_hosts.Size()<50){
		DeleteFile("c:\\AresProtector\\hosts.dat");  //get rid of this pos since it became corrupt somehow.  For simplicity we'll just wait for next reboot to load from synched host cache
	}
}*/

AresHost* AresHostCache::GetNextHost(void)
{
	CSingleLock lock(&m_lock,TRUE);

	if(m_host_offset>=(int)mv_hosts.Size())
		m_host_offset=0;

	if(mv_hosts.Size()==0)
		return NULL;

	return (AresHost*)mv_hosts.Get(m_host_offset++);
}

void AresHostCache::AddHosts(Vector& v_hosts)
{
	//mv_new_hosts.Append(&v_hosts,1);
	/*
	for(int i=0;i<(int)v_hosts.Size();i++){
		AresHost *host=(AresHost*)v_hosts.Get(i);
		if(mv_hosts.BinaryFind(host)!=-1 || mv_new_hosts.BinaryFind(){
			v_hosts.Remove(i);
			i--;
		}
	}

	if(v_hosts.Size()==0)  //were they all duplicates?
		return;

	mv_hosts.Append(&v_hosts,1);
	mv_hosts.Sort(1);

	//randomly remove some until the size is less than X hosts
	while(mv_hosts.Size()>3000){
		mv_hosts.Remove(rand()%mv_hosts.Size());
	}

	if(m_last_save.HasTimedOut(60*5) || !mb_has_saved_once){  //save at most once every 5 minutes to avoid wasteful disk access
		mb_has_saved_once=true;
		WriteOutHostsFile();
		m_last_save.Refresh();
	}*/
}

/*
void AresHostCache::WriteOutHostsFile(void)
{
	CreateDirectory("c:\\AresProtector",NULL);

	HANDLE file = CreateFile("c:\\AresProtector\\hosts.dat",			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                0,							// we don't like to share
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	if(file==INVALID_HANDLE_VALUE || file==NULL){
		//something is seriously wrong
		return;
	}

	int len=mv_hosts.Size();
	for(int i=0;i<len;i++){
		((AresHost*)mv_hosts.Get(i))->WriteToFile(file);
	}

	TRACE("AresHostCache::WriteOutHostsFile(void) %d hosts have been written to file \"c:\\AresProtector\\hosts.dat\".\n",mv_hosts.Size()); 

	CloseHandle(file);
}
*/
void AresHostCache::Update(void)
{
	//TYDEBUG
	/*
	static Timer last_update;
	static Timer last_save;

	if(last_save.HasTimedOut(60*30)){
	//	AresDataCollectorSystemReference ref;

	//	ref.System()->Log("Host Cache:  Saving hosts to file...");
		WriteOutHostsFile();
	//	ref.System()->Log("Host Cache:  Finished saving hosts to file.");
		last_save.Refresh();
	}

	if(last_update.HasTimedOut(2*60)){
		
		if(mv_new_hosts.Size()>50){  //only run code if there is significant work to do
			//AresDataCollectorSystemReference ref;
			int orig_new_hosts=mv_new_hosts.Size();

			//CString log_str;
			//log_str.Format("Host Cache:  Attempting to merge %d new hosts...",orig_new_hosts);
			//ref.System()->Log(log_str);
			Vector v_tmp;
			v_tmp.Copy(&mv_hosts);
			
			v_tmp.Append(&mv_new_hosts);

			mv_new_hosts.Clear();
			v_tmp.Sort(1);
			for(int i=0;i<(int)v_tmp.Size()-1;i++){  //purge duplicate entries, they should be adjacent since the vector is sorted
				AresHost* ah1=(AresHost*)v_tmp.Get(i);
				AresHost* ah2=(AresHost*)v_tmp.Get(i+1);
				if(ah1->GetIntIP()==ah2->GetIntIP()){
					v_tmp.Remove(i+1);
					i--;
				}
			}


			
			CSingleLock lock(&m_lock,TRUE);  //save the lock towards the end so we don't block the GUI thread very much

			int old_size=mv_hosts.Size();
			mv_hosts.Copy(&v_tmp);

			
//			log_str.Formst("Host Cache: Finished merging, %d out of %d new hosts successfully merged.",mv_hosts.Size()-old_size,orig_new_hosts);
//			ref.System()->Log(log_str);
			int purged=0;
			while(mv_hosts.Size()>4000){  //if our vector of hosts is getting large, then purge it
				//chose a random section to delete
				int purge_index=rand()%(mv_hosts.Size());
				mv_hosts.Remove(purge_index);
				purged++;
			}
			if(purged>0){
				//log_str.Format("Host Cache:  Purged %d hosts due to oversizing.",purged);
				//ref.System()->Log(log_str);
			}
		}

		last_update.Refresh();
	}
	*/
}

UINT AresHostCache::Run(void)
{
	Timer last_reload;
	bool b_has_loaded=false;
	srand(GetTickCount());
	int delay=10+(rand()%LOADDELAY);  //TYDEBUG


	while(!this->b_killThread){
		Sleep(100);

	
		if((!b_has_loaded && last_reload.HasTimedOut(delay)) || (last_reload.HasTimedOut(60*60*12)) || (mv_hosts.Size()==0 && last_reload.HasTimedOut(60*60*2))){
			CString log_msg;
			b_has_loaded=true;
			TRACE("AresHostCache::Run(void) QUERY HOSTS BEGIN\n");
			//AresSupernodeSystemRef ref;
			//ref.System()->LogToFile("AresHostCache::Run(void) QUERY HOSTS BEGIN");
			//ref.System()->LogQuery("Loading hosts...");
			last_reload.Refresh();

			int offset=0;
			
			TinySQL sql;
			if(this->b_killThread || !sql.Init("206.161.141.35","onsystems","ebertsux37","ares_data",3306)){
				//ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS END 0");
				TRACE("AresHostCache::Run(void) QUERY HOSTS END, QUERY FAIL\n");
				continue;
			}

//			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS POSITION A");


////IMPORT
			if(!sql.Query("select count(*) from ares_hosts",true)){  //TYDEBUG
				log_msg.Format("FAILED to query database for hosts because %s. (will retry later) ",sql.m_fail_reason.c_str());
				TRACE("%s\n",(LPCSTR)log_msg);
//				ref.System()->LogQuery(log_msg);
//				ref.System()->LogToFile("AresSNHostCache::Run() END 1");
				continue;
			}

			if(sql.m_num_rows<1){
				log_msg.Format("FAILED to query database for hosts because Corrupt Count. (will retry later) ");
				TRACE("%s\n",(LPCSTR)log_msg);
//				ref.System()->LogQuery(log_msg);
//				ref.System()->LogToFile("AresSNHostCache::Run() END 1");
				continue;
			}

			int count=atoi(sql.mpp_results[0][0].c_str());

			int NUM_HOSTS=50000;

			offset=rand()%(max(1,max(1,count-(NUM_HOSTS/3))/100));
			offset*=100;  //offset was scaled down by 100 because rand() only goes up to 65k
		
			offset=max(0,offset-(NUM_HOSTS/3)); //we do this so that we have a good chance of selecting the ones in front and the back, since we are randoming for a range.  If we didn't do this there would be about 1/150000 chance that the start would be picked, not good
			offset=min(offset,max(0,count-(NUM_HOSTS)));

			CString query;
			query.Format("SELECT ip,port from ares_hosts limit %u,%u",offset,NUM_HOSTS);

			if(!sql.Query(query,true)){  //TYDEBUG
				log_msg.Format("FAILED to query database for hosts because %s. (will retry later) ",sql.m_fail_reason.c_str());
				TRACE("%s\n",(LPCSTR)log_msg);
				//ref.System()->LogQuery(log_msg);
				//ref.System()->LogToFile("AresSNHostCache::Run() END 1");
				continue;
			}
			
////END IMPORT


//			if(this->b_killThread || !sql.Query("SELECT ip,port from ares_hosts order by rand() limit 15000",true)){
//				TRACE("AresSNHostCache::Run(void) QUERY HOSTS END, QUERY FAIL\n");
//				ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS END 1");
//				continue;
//			}

			//ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS POSITION B");
			Vector v_tmp;

			Vector v_tmp2;

			for(int i=0;i<(int)sql.m_num_rows;i++){
				AresHost *as=new AresHost(sql.mpp_results[i][0].c_str(),atoi(sql.mpp_results[i][1].c_str()));
				if(as->GetPort()==4685)
					v_tmp2.Add(as);
				else
					v_tmp.Add(as);
			}

//			ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS POSITION C");
			CSingleLock lock(&m_lock,TRUE);
			CSingleLock lock2(&m_md_host_lock,TRUE);
			mv_hosts.Copy(&v_tmp);
			mv_md_hosts.Copy(&v_tmp2);
			
			m_host_offset=rand()%max(1,mv_hosts.Size());  //set this one to be a random offset

			//ref.System()->LogToFile("AresSNHostCache::Run() Position B");
			//log_msg.Format("Loaded %u supernodes from the database with start_offset=%u",mv_hosts.Size(),offset);
			
			//ref.System()->LogQuery(log_msg);

			b_has_loaded=true;

			//ref.System()->LogToFile("AresSNHostCache::Run(void) QUERY HOSTS END");
			TRACE("AresHostCache::Run(void) QUERY HOSTS END\n");
			mb_ready=true;
		}
	}
	return 0;
}

void AresHostCache::StartSystem(void)
{
	this->StartThreadLowPriority();
}

void AresHostCache::StopSystem(void)
{
	this->StopThread();
}

bool AresHostCache::IsReady(void)
{
	return mb_ready;
}
