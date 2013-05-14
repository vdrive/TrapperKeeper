#include "StdAfx.h"
#include "aresdchostcache.h"
#include <mmsystem.h>
#include "AresDataCollectorSystem.h"
#include "..\tkcom\TinySQL.h"

AresDCHostCache::AresDCHostCache(void)
{
	mb_has_saved_once=false;
	m_host_offset=0;
	
}

AresDCHostCache::~AresDCHostCache(void)
{
}

void AresDCHostCache::ReadInHostFile(void)
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
			TRACE("AresDCHostCache::ReadInHostFile(void) failed because it couldn't open the file.  Error code = %d\n",error);
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
	
	TRACE("AresDCHostCache::ReadInHostFile(void) added %d hosts.\n",mv_hosts.Size()); 

	delete [] data;
	CloseHandle(file);
}

AresHost* AresDCHostCache::GetNextHost(void)
{
	CSingleLock lock(&m_lock,TRUE);

	if(m_host_offset>=(int)mv_hosts.Size())
		m_host_offset=0;

	if(mv_hosts.Size()==0)
		return NULL;

	return (AresHost*)mv_hosts.Get(m_host_offset++);
}

void AresDCHostCache::AddHosts(Vector& v_hosts)
{
	mv_new_hosts.Append(&v_hosts,1);
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

void AresDCHostCache::WriteOutHostsFile(void)
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

	TRACE("AresDCHostCache::WriteOutHostsFile(void) %d hosts have been written to file \"c:\\AresProtector\\hosts.dat\".\n",mv_hosts.Size()); 

	CloseHandle(file);
}

void AresDCHostCache::Update(void)
{
	
	static Timer last_update;
	static Timer last_save;

	if(last_save.HasTimedOut(60*30)){
		AresDataCollectorSystemReference ref;

		ref.System()->Log("Host Cache:  Saving hosts to file...");
		WriteOutHostsFile();
		ref.System()->Log("Host Cache:  Finished saving hosts to file.");
		last_save.Refresh();
	}

	if(last_update.HasTimedOut(60)){
		
		if(mv_new_hosts.Size()>50){  //only run code if there is significant work to do
			AresDataCollectorSystemReference ref;
			int orig_new_hosts=mv_new_hosts.Size();

			CString log_str;
			log_str.Format("Host Cache:  Attempting to merge %d new hosts...",orig_new_hosts);
			ref.System()->Log(log_str);
			Vector v_tmp;
			v_tmp.Copy(&mv_hosts);
			
		
			vector<string> v_ips;
			vector<unsigned short> v_ports;

			//send these new hosts to our server
			for(int i=0;i<(int)mv_new_hosts.Size();i++){ 
				AresHost* ah1=(AresHost*)mv_new_hosts.Get(i);
				v_ips.push_back(ah1->GetIP());
				v_ports.push_back(ah1->GetPort());

				if(v_ips.size()>45){  //only send up to X at a time
					ref.System()->GetSupernodeServerSystemInterface()->ReportNewSupernodesToServer(v_ips,v_ports);
					v_ips.clear();
					v_ports.clear();
				}
			}

			if(v_ips.size()>0){
				ref.System()->GetSupernodeServerSystemInterface()->ReportNewSupernodesToServer(v_ips,v_ports);
			}

			/*
			if(ref.System()->IsController()){
				//if this is the controller rack, lets inject our hosts into a table so we can look at them

				int count=0;
				CString query="insert ignore into ares_hosts values ";
				TinySQL sql;
				sql.Init(DATABASE_ADDRESS,"onsystems","ebertsux37","ares_data",3306);

				for(int i=0;i<(int)mv_new_hosts.Size() && count<600;i++){ 
					AresHost* ah1=(AresHost*)mv_new_hosts.Get(i);
					count++;
					CString tmp;
					if(i>0)
						tmp.Format(",('%s',%d,now())",ah1->GetIP(),ah1->GetPort());
					else
						tmp.Format("('%s',%d,now())",ah1->GetIP(),ah1->GetPort());

					query+=tmp;

					CString tmp_query;
					tmp_query.Format("update ares_hosts set time_created=now() where ip='%s' and port=%u",ah1->GetIP(),ah1->GetPort());
					sql.Query(tmp_query,false);
				}


				sql.Query(query,false);
			}*/
			

			v_tmp.Append(&mv_new_hosts);

			mv_new_hosts.Clear();
			v_tmp.Sort(1);
			for(int i=0;i<(int)v_tmp.Size()-1;i++){  //purge duplicate entries, they should be adjacent since the vector is sorted
				AresHost* ah1=(AresHost*)v_tmp.Get(i);
				AresHost* ah2=(AresHost*)v_tmp.Get(i+1);
				if(ah1->GetIntIP()==ah2->GetIntIP()){

					if(ah2->m_fail_count>ah1->m_fail_count)
						ah1->m_fail_count=ah2->m_fail_count;
					v_tmp.Remove(i+1);
					i--;
					continue;
				}
				if(ah1->m_fail_count>20 && v_tmp.Size()>1000){
					v_tmp.Remove(i);
					i--;
					continue;
				}
			}

			CSingleLock lock(&m_lock,TRUE);  //save the lock towards the end so we don't block the GUI thread very much

			int old_size=mv_hosts.Size();
			mv_hosts.Copy(&v_tmp);

			log_str.Format("Host Cache: Finished merging, %d out of %d new hosts successfully merged.",mv_hosts.Size()-old_size,orig_new_hosts);
			ref.System()->Log(log_str);
			int purged=0;
			while(mv_hosts.Size()>4000){  //if our vector of hosts is getting large, then purge it
				//chose a random section to delete
				int purge_index=rand()%(mv_hosts.Size());
				mv_hosts.Remove(purge_index);
				purged++;
			}
			if(purged>0){
				log_str.Format("Host Cache:  Purged %d hosts due to oversizing.",purged);
				ref.System()->Log(log_str);
			}
		}

		last_update.Refresh();
	}
}

UINT AresDCHostCache::Run(void)
{
	AresDataCollectorSystemReference ref;
	ref.System()->Log("Host Cache: Thread Started");
	ref.System()->Log("Host Cache: Loading host list...");
	ReadInHostFile();
	CString log_msg;
	log_msg.Format("Host Cache: Finished loading %d hosts from file",mv_hosts.Size());
	ref.System()->Log(log_msg);
	while(!this->b_killThread){
		Sleep(100);
		Update();
	}
	return 0;
}
