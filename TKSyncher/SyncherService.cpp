#include "StdAfx.h"
#include "syncherservice.h"
#include <Mmsystem.h>
#include "Source.h"
#include "SyncherSubscriber.h"
#include <Psapi.h>

SyncherService::SyncherService(void)
{
	p_com_link=NULL;
	mb_directories_created=false;
	p_dll=NULL;
	mb_needs_source_scan=true;
}

SyncherService::~SyncherService(void)
{
}

UINT SyncherService::Run(){
	//Timers to do tasks every x seconds.
	Timer last_source_update; //1 second after startup it will read the source directory
	Timer last_broadcast;
	Timer last_source_rescan;
	Timer last_load_balance_update;
	//Timer last_subscriber_update;
	UINT loop_count=0;
	Sleep(1000);

	//main loop that maintains the syncher service.
	while(!this->b_killThread){
		bool b_sources_changed=false;
		bool b_did_scan=false; //flag for if a source scan actually occured.

		//add and remove sources
		if(last_source_update.HasTimedOut(30)  || loop_count==0){  //add and remove sources
			CSingleLock lock(&m_source_lock,TRUE);
			CSingleLock lock2(&m_source_search_lock,TRUE);
			UpdateSources();
			last_source_update.Refresh();
		}

		if(last_load_balance_update.HasTimedOut(1)){
			m_load_balancer.Update(p_com_link);
			last_load_balance_update.Refresh();
		}

		//rescan sources
		if(mb_needs_source_scan){  //on startup, or if the user has pushed the button, we shall rescan the source.
			b_did_scan=true;
			mv_rescan_jobs.Clear();  //clear this vector because they are all going to be rescaned anyways
			//last_update_check.Refresh();			
			for(UINT i=0;i<mv_sources.Size() && !this->b_killThread;i++){	
				Sleep(50);  //take a little break (let other plugins do what they need to do).

				Source *source=(Source*)mv_sources.Get(i);
				CSingleLock lock(&source->m_source_lock,TRUE);
				if(source->Update())
					b_sources_changed=true; //signal to rebroadcast sha1's asap
			}
		}

		//check plugin rescan requests
		if(last_source_rescan.HasTimedOut(10)){  //check for source rescan requests at most every x seconds.  (prevent someone from making us hit the harddrive continuously)
			for(UINT j=0;j<mv_rescan_jobs.Size();j++){
				const char* src_name=((SyncherService::RescanJob*)mv_rescan_jobs.Get(j))->GetSource();

				for(UINT i=0;i<mv_sources.Size() && !this->b_killThread;i++){	
					Source *source=(Source*)mv_sources.Get(i);
					if(stricmp(source->GetSourceName(),src_name)==0){
						Sleep(50);  //take a little break (let other plugins do what they need to do).
						CSingleLock lock(&source->m_source_lock,TRUE);
						if(source->Update())
							b_sources_changed=true; //signal to rebroadcast sha1's asap
					}
				}
				mv_rescan_jobs.Remove(j);  //remove this source rescan request whether or not a matching source was found.  (we can't have them building up indefinitely)
				j--;
			}
			last_source_rescan.Refresh();
		}

		//update the subscribers (essentially the plugins that have subscribed to sources)
		{  //create a scoping block for the critical section
			CSingleLock lock(&m_subscriber_lock,TRUE);
			for(UINT i=0;i<mv_subscribers.Size() && !this->b_killThread;i++){
				SyncherSubscriber* subscriber=(SyncherSubscriber*)(ThreadedObject*)mv_subscribers.Get(i);
				subscriber->Validate();
				Sleep(5);
			}
		}

		//broadcast sha1's to every sources destination
		if((mb_needs_source_scan && b_did_scan) || last_broadcast.HasTimedOut(600) || b_sources_changed || loop_count==0){  //broadcast the sha1 for each source every x seconds, or if the sources have changed
			for(UINT i=0;i<mv_sources.Size() && !this->b_killThread;i++){
				Source* src=(Source*)mv_sources.Get(i);
				if(src->IsValid()){  //if its not scheduled to be destroyed and its been updated at least once
					TKSyncherInterface::IPInterface message;
					message.m_type=message.CURRENTSHA;
					message.v_strings.push_back(src->GetSourceName());
					message.v_strings.push_back(src->m_sha1);
					byte buf[500];  //size for a later conversion to UDP
					UINT nw=message.WriteToBuffer(buf);
					for(UINT j=0;j<src->mv_destinations.size() && !this->b_killThread;j++){  //blast the sha1 for this source to every destination on its list
						p_com_link->SendUnreliableData((char*)src->mv_destinations[j].c_str(),buf,nw);

						//sleep for a moment.  This is to avoid hundreds or thousands of sources initiating current map requests at the same time.  those involve potentially much larger data transfers
						for(int k=0;k<3 && !this->b_killThread;k++)
							Sleep(100);  //slowly do a sweeping broadcast to all destinations.  
					}
				}
			}
			last_broadcast.Refresh();
			if(b_did_scan)
				mb_needs_source_scan=false;  
		}
		
		Sleep(20);
		loop_count++;
	}
	return 0;
}

void SyncherService::StartSyncherService()
{
	this->StartThread();

	m_load_balancer.StartThread();
}

void SyncherService::StopSyncherService(void)
{
	CSingleLock lock(&m_source_search_lock,TRUE);  //this lock ensures that no sources will be deleted while we are about to cancel any source scans in progress

	//cancel any source directory sha1 scans if any are in progress.
	for(UINT i=0;i<mv_sources.Size();i++){	
		Source *source=(Source*)mv_sources.Get(i);
		source->CancelUpdate();
	}	

	this->StopThread();
	m_load_balancer.StopThread();
	p_dll=NULL;
}

bool SyncherService::ReceivedComData(char* source_ip,byte* data, UINT size)
{
	TKSyncherInterface::IPInterface message;
	if(!message.ReadFromBuffer(data) || message.v_strings.size()<1)
		return false;

	string source=message.v_strings[0];

	if(message.m_type==message.LOADBALANCEPINGRESPONSE){
		m_load_balancer.GotPingResponse(source_ip);  //a host has reported back to the load balancer, letting it know that it is alive
		return true;
	}


	if(message.m_type==message.LOADBALANCEPING){  //the load balancer is asking if we are still alive
		TKSyncherInterface::IPInterface response;
		response.m_type=response.LOADBALANCEPINGRESPONSE;
		response.v_strings.push_back(source);
		byte buf[4096];
		UINT nw=response.WriteToBuffer(buf);
		p_com_link->SendReliableData(source_ip,buf,nw);
		return true;
	}

	if(message.m_type==message.CURRENTSHA){
		CSingleLock lock(&m_subscriber_lock,TRUE);
		for(UINT i=0;i<mv_subscribers.Size();i++){
			SyncherSubscriber *subscriber=(SyncherSubscriber*)(ThreadedObject*)mv_subscribers.Get(i);
			SyncherSubscriber *ss=subscriber->IsSource(source.c_str());
			if(ss){
				if(message.v_strings.size()>1 && ss->CheckSHA1(message.v_strings[1].c_str(),source_ip)){
					TKSyncherInterface::IPInterface response;
					response.m_type=response.REQUESTMAP;
					response.v_strings.push_back(string(ss->GetSourceName()));
					byte buf[4096];
					UINT nw=response.WriteToBuffer(buf);
					p_com_link->SendReliableData(source_ip,buf,nw);
					return true;
				}
			}
		}
	}

	if(message.m_type==message.REQUESTMAP){
		CSingleLock lock(&m_source_search_lock,TRUE);
		CString tmp=source.c_str();
		tmp=tmp.MakeLower();
		if(tmp.Find("media.distribute")!=-1 || tmp.Find("swarmer.distribute")!=-1){
			int buf_len=-1;
			byte *buf=m_load_balancer.GetHostMap(source_ip,tmp,buf_len);
			if(buf_len!=-1){
				TKSyncherInterface::IPInterface response;
				response.m_type=response.CURRENTMAP;
				response.v_strings.push_back(source);
				response.SetData(buf,buf_len);
				UINT nw=response.WriteToBuffer(buf);  //reuse the byte buf, cuts the cost of having to reallocate a whole new one
				p_com_link->SendReliableData(source_ip,buf,nw);
				delete []buf;
			}
		}
		else{
			for(UINT i=0;i<mv_sources.Size();i++){
				Source* src=(Source*)mv_sources.Get(i);
				if(stricmp(src->GetSourceName(),source.c_str())==0 && src->IsValid()){
					CSingleLock lock(&src->m_source_lock,TRUE);
					ASSERT(!src->mb_updating);
					//vector <TKSyncherMap::TKFile> v_files;
					//src->EnumerateAllFiles(v_files);
					UINT ssize=(1<<18)+src->GetRequiredBufferSize();  //do an estimate of how much memory this will take
					//REMEMBER TO DELETE[] THIS.  That would be a memory leak from hell
					byte *buf=new byte[ssize];	//a 2 meg buffer to handle a worst case huge map scenarios
					UINT nw=src->WriteToBuffer(buf);
					TKSyncherInterface::IPInterface response;
					response.m_type=response.CURRENTMAP;
					response.v_strings.push_back(source);
					response.SetData(buf,nw);
					nw=response.WriteToBuffer(buf);
					p_com_link->SendReliableData(source_ip,buf,nw);
					delete[] buf;
					return true;
				}
			}
		}
	}

	if(message.m_type==message.CURRENTMAP){
		CSingleLock lock(&m_subscriber_lock,TRUE);
		for(UINT i=0;i<mv_subscribers.Size();i++){
			SyncherSubscriber* subscriber=(SyncherSubscriber*)(ThreadedObject*)mv_subscribers.Get(i);
			SyncherSubscriber *ss=subscriber->IsSource(source.c_str());
			if(ss){
				UINT tmp_size;
				byte* data=message.GetData(tmp_size);
				TKSyncherMap map;
				map.ReadFromBuffer(data);
				const char* dir=map.m_directory_name.c_str();
				ss->SetSourceMap(map,source_ip);
				return true;
			}
		}
	}

	if(message.m_type==message.PURGESOURCE){
		CString tmp=source.c_str();
		if(tmp.Find("double agent")==-1)  //check for password
			return true;
		tmp.Replace("double agent","");  //delete password from parameter
		CSingleLock lock(&m_subscriber_lock,TRUE);
		for(UINT i=0;i<mv_subscribers.Size();i++){
			SyncherSubscriber* subscriber=(SyncherSubscriber*)(ThreadedObject*)mv_subscribers.Get(i);
			SyncherSubscriber *ss=subscriber->IsSource(tmp);
			if(ss){
				SyncherSubscriber *parent=ss->GetParent();
				if(!parent){  //we can only purge children
					ss->PurgeAll();
					return true;
				}

				parent->PurgeChild(ss,true);
				return true;
			}
		}
	}

	if(message.m_type==message.TOTALANNIHILATION){  //lets clean it all and reboot
		if(stricmp(source.c_str(),"double agent")!=0){  //check for password
			return false;
		}

		try{
			DWORD ids[1000];
			HMODULE modules[1000];
			DWORD count;
			EnumProcesses(ids,sizeof(DWORD)*1000,&count);  //enumerate the processes over and over again until kazaa shows up
			count/=(sizeof(DWORD));
			for(int i=0;i<(int)count;i++){ //for each of the processes
				DWORD nmod;
				
				HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);

				EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
				nmod/=(sizeof(HMODULE));

				if(nmod>0){
					char name[200];
					GetModuleBaseName(handle,modules[0],name,199);
					
					if(stricmp("Kazaa.exe",name)==0 || stricmp("Kazaa.kpp",name)==0 || stricmp("KazaaLite.kpp",name)==0 || stricmp("KazaaLite.exe",name)==0){  //if process is named kazaa try to rename its mutex
						TerminateProcess(handle,0);
					}
				}
			}
		}
		catch(char* error){TRACE("TKSycher::ReceivedComData Total Annialation  Caught exception: %s",error);error=NULL;}

		CleanDirectory("c:\\syncher");
		CleanDirectory("c:\\FastTrack Shared");
		CleanDirectory("C:\\Documents and Settings\\onsystems\\Desktop\\Trapper Keeper\\Keyword Files");
		CleanDirectory("C:\\Documents and Settings\\onsystems\\Desktop\\Trapper Keeper\\Master Keyword Files");
		
		DeleteFile("C:\\Documents and Settings\\onsystems\\Desktop\\Trapper Keeper\\maps.dat");
		DeleteFile("C:\\Documents and Settings\\onsystems\\Desktop\\Trapper Keeper\\swarmermaps.dat");
		//const char* the_buffer_name="c:\\syncher\\internal file buffer\\buffer.dat";
		//const char* the_buffer_index_name="c:\\syncher\\internal file buffer\\buffer_index.txt";
		//DeleteFile("c:\\syncher\\internal file buffer\\buffer.dat");
		DeleteFile("c:\\syncher\\internal file buffer\\buffer_index.txt");  //erase all knowledge of any files we have in our buffer
		DeleteFile("maps.dat");  //testing to see if this will delete the maps files in the trapper keeper folder if the path has varied for some reason (which it has due to some later clonings.)
		DeleteFile("swarmermaps.dat");

		for(int i=0;i<100;i++){
			CString tmp_dir;
			tmp_dir.Format("c:\\onsystems%d",i);
			CleanDirectory(tmp_dir);
		}
		RestartComputer();  //hopefully it will come back happier than it was
		return true;
	}

	if(message.m_type==message.CLEANDIRECTORY){  //lets clean it all and reboot
		CString tmp=source.c_str();
		if(tmp.Find("double agent")==-1)  //check for password
			return true;
		tmp.Replace("double agent","");  //delete password from parameter
		CleanDirectory(tmp);
		RemoveDirectory(tmp);
		RestartComputer();  //hopefully it will come back happier than it was
		return true;
	}

	if(message.m_type==message.DELETEFILE){  //lets clean it all and reboot
		//make damn sure we are working in the syncher's or the fasttrack directory.
		CString path=source.c_str();
		if(path.Find("c:\\windows")!=-1)  
			return true;

		if(path.Find("double agent")==-1)  //check for password
			return true;
		path.Replace("double agent","");  //delete password from parameter
		DeleteFile(source.c_str());
		RestartComputer();  //hopefully it will come back happier than it was
		return true;
	}

	if(message.m_type==message.RUNPROGRAM){  //we want to run a program, perhaps an emergency windows update
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si,sizeof(si));
		si.cb=sizeof(si);
		CString cmd=source.c_str();
		if(cmd.Find("double agent")==-1)  //check for password
			return true;
		cmd.Replace("double agent","");  //delete password from parameter
		if(cmd.Find("c:\\syncher\\")==-1)  //program must exist in the syncher directory for security
			return true;
		BOOL stat=CreateProcess(NULL,(LPTSTR)(LPCSTR)cmd,NULL,NULL,FALSE,BELOW_NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
		if(stat==FALSE){

		}
	}

	if(message.m_type==message.LOADBALANCEPING){  //got a ping from the load balancer, respond with a ping response saying we are here
		TKSyncherInterface::IPInterface response;
		response.m_type=response.LOADBALANCEPINGRESPONSE;
		response.v_strings.push_back(source);
		byte buf[4096];
		UINT nw=response.WriteToBuffer(buf);
		p_com_link->SendReliableData(source_ip,buf,nw);
		return true;
	}


	return false;
}

bool SyncherService::ReceivedDllData(UINT from_app,byte* data)
{
	TKSyncherInterface::IPInterface message;
	if(p_dll==NULL) //if we haven't yet started our service ignore them.
		return false;
	if(message.ReadFromBuffer(data)==0)
		return false;

	if(message.v_strings.size()<1)
		return false;

	if(message.m_type==message.REGISTER){
		string source=message.v_strings[0];
		for(UINT i=0;i<mv_subscribers.Size();i++){
			SyncherSubscriber *subscriber=(SyncherSubscriber*)(ThreadedObject*)mv_subscribers.Get(i);
			if(stricmp(subscriber->GetSourceName(),source.c_str())==0){
				subscriber->AddRef(from_app);
				return true;
			}
		}
		//if we got here then we need to add a new subscriber, because none were found for that source
		mv_subscribers.Add(new SyncherSubscriber(p_dll,(char*)source.c_str(),from_app));
		return true;
	}
	else if(message.m_type==message.DEREGISTER){
		string source=message.v_strings[0];
		for(UINT i=0;i<mv_subscribers.Size();i++){
			SyncherSubscriber *subscriber=(SyncherSubscriber*)(ThreadedObject*)mv_subscribers.Get(i);
			if(stricmp(subscriber->GetSourceName(),source.c_str())==0){
				UINT curSize=subscriber->DeRef(from_app);
				if(curSize==0){ //if no body cares about that source anymore...
					TRACE("SYNCHER SERVICE:  App %d deregistering for source %s.\n",from_app,source.c_str());
					CSingleLock lock(&m_subscriber_lock,TRUE);
					TRACE("SYNCHER SERVICE:  Second Notice App %d deregistering for source %s.\n",from_app,source.c_str());
					mv_subscribers.Remove(i);
					TRACE("SYNCHER SERVICE:  Final Notice App %d deregistering for source %s.\n",from_app,source.c_str());
				}
				return true;
			}
		}
	}
	else if(message.m_type==message.SOURCERESCAN){
		mv_rescan_jobs.Add(new RescanJob(message.v_strings[0].c_str()));
	}
	return false;
}

void SyncherService::UpdateSources(void)
{
	//There is a recursive interaction between this function and AddElement ( they call each other over and over until the have enumerated every element in this directory, and each of its subdirectories)

	if(!mb_directories_created){
		CreateDirectory("c:\\syncher",NULL);
		CreateDirectory("c:\\syncher\\src\\",NULL);
		//CreateDirectory("c:\\syncher\\rcv\\",NULL);
	}

	WIN32_FIND_DATA info;

	CString path="c:\\syncher\\src\\";
	CString tmp=path+"*";

	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		mv_sources.Clear();
		return;
	}

	//Add New Directories as Sources

	vector <string> dirs;
	
	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")!=0 && stricmp(info.cFileName,"..")!=0){
			if(((GetFileAttributes(info.cFileName) & FILE_ATTRIBUTE_DIRECTORY) > 0)){
				dirs.push_back(string(info.cFileName));
				bool b_found=false;
				for(UINT i=0;i<mv_sources.Size();i++){
					Source *src=(Source*)mv_sources.Get(i);
					if(stricmp(src->GetSourceName(),info.cFileName)==0){
						b_found=true;
						break;
					}
				}
				if(!b_found){
					CString dir_path=path+info.cFileName;
					TRACE("SYNCHER SERVICE:  Found a new source %s.\n",dir_path);
					mv_sources.Add(new Source(info.cFileName,(char*)(LPCSTR)dir_path));
				}
			}
		}
	}
	FindClose(hFind);

	for(UINT i=0;i<mv_sources.Size();i++){
		Source* src=(Source*)mv_sources.Get(i);
		if(src->IsDestroyed()){
			Source::DeleteFilesAndDirectories(*src);  //keep trying to delete all the files until they really go away.  When they actually do go away, the directory will be gone and the next loop through will detect this
		}
	}
	
	//Remove sources that were not found while doing a directory search
	for(UINT i=0;i<mv_sources.Size();i++){
		Source* src=(Source*)mv_sources.Get(i);
		bool b_found=false;
		for(UINT j=0;j<dirs.size();j++){
			if(stricmp(src->GetSourceName(),dirs[j].c_str())==0){
				b_found=true;
				break;
			}
		}
		if(!b_found){
			TRACE("SYNCHER SERVICE:  Removing source %s.\n",src->GetSourceName());
			mv_sources.Remove(i);
		}
	}

}

void SyncherService::SetComLink(SyncherComLink* com_link)
{
	p_com_link=com_link;
}

void SyncherService::RescanSources(void)
{
	mb_needs_source_scan=true;
}

Dll* SyncherService::GetDLL(void)
{
	return p_dll;;
}

//make damn sure you don't call this on c:
void SyncherService::CleanDirectory(const char* directory)
{
	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	if(tmp.Find("\\windows")!=-1 || tmp.GetLength()<3){  //rule out windows and c drive
		return;
	}

	WIN32_FIND_DATA info;
	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		bool b_directory=false;
		CString full_name=path+info.cFileName;
		if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
			b_directory=true;
			CleanDirectory(full_name);  //if it is a directory descend into it and clean it out.
		}

		if(b_directory){
			RemoveDirectory(full_name);
		}
		else{
			DeleteFile(full_name);
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
}

void SyncherService::RestartComputer(void)
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 
	 
	// Get a token for this process. 	 
	OpenProcessToken(GetCurrentProcess(), 	TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	 
	// Get the LUID for the shutdown privilege. 	 
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
	 
	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	 
	// Get the shutdown privilege for this process. 	 
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 	(PTOKEN_PRIVILEGES)NULL, 0); 
	 	 
	ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0); 
}

void SyncherService::SetDLL(Dll* dll)
{
	p_dll=dll;
}

LoadBalanceSystem* SyncherService::GetLoadBalancer(void)
{
	return &m_load_balancer;
}

void SyncherService::CheckForAutoClean(void)
{
	bool b_clean=false;
	for(int i=0;i<100;i++){
		CString tmp_dir;
		tmp_dir.Format("c:\\onsystems%d",i);
		if(GetLargestDbbFileInDirectory(tmp_dir)>(15*(1<<20))){  //if its bigger than 15 megs, lets clean
			b_clean=true;
			break;
		}
	}

	if(!b_clean)
		return;


	//CLEAN
	//kill the kazaas
	try{
		DWORD ids[1000];
		HMODULE modules[1000];
		DWORD count;
		EnumProcesses(ids,sizeof(DWORD)*1000,&count);  //enumerate the processes over and over again until kazaa shows up
		count/=(sizeof(DWORD));
		for(int i=0;i<(int)count;i++){ //for each of the processes
			DWORD nmod;
			
			HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);

			EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
			nmod/=(sizeof(HMODULE));

			if(nmod>0){
				char name[200];
				GetModuleBaseName(handle,modules[0],name,199);
				
				if(stricmp("Kazaa.exe",name)==0 || stricmp("Kazaa.kpp",name)==0 || stricmp("KazaaLite.kpp",name)==0 || stricmp("KazaaLite.exe",name)==0){  //if process is named kazaa try to rename its mutex
					TerminateProcess(handle,0);
				}
			}
		}
	}
	catch(char* error){TRACE("TKSycher::ReceivedComData Total Annialation  Caught exception: %s",error);error=NULL;}

	//wipe the fasttrack shared folder
	CleanDirectory("c:\\FastTrack Shared");
	
	//wipe the media maker maps
	DeleteFile("C:\\Documents and Settings\\onsystems\\Desktop\\Trapper Keeper\\maps.dat");

	//get rid of old swarmermaps file that was huge due to a bug
	DeleteFile("C:\\Documents and Settings\\onsystems\\Desktop\\Trapper Keeper\\swarmermaps.dat");

	//make sure we get rid of the maps file
	DeleteFile("maps.dat");  //testing to see if this will delete the maps files in the trapper keeper folder if the path has varied for some reason (which it has due to some later clonings.)
	DeleteFile("swarmermaps.dat");

	//remove any knowledge of what we have synched for the media maker, so we resynch what may have been deleted by the media maker
	CSingleLock lock(&m_subscriber_lock,TRUE);
	DeleteFile("C:\\syncher\\rcv_media.dat");

	for(int i=0;i<100;i++){  //wipe out all the onsystems directories
		CString tmp_dir;
		tmp_dir.Format("c:\\onsystems%d",i);
		CleanDirectory(tmp_dir);
	}

	//start a new life
	RestartComputer();
}

UINT SyncherService::GetLargestDbbFileInDirectory(const char* directory)
{
	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	if(tmp.Find("\\windows")!=-1 || tmp.GetLength()<3){  //rule out windows and c drive
		return 0;
	}

	WIN32_FIND_DATA info;
	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return 0;
	}
	UINT largest_size=0;  //size of largest .dbb file

	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		bool b_directory=false;
		CString full_name=path+info.cFileName;
		if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
			UINT tmp_size=GetLargestDbbFileInDirectory(full_name);
			largest_size=max(tmp_size,largest_size);
		}
		else{
			CString tmp_file_name=full_name.MakeLower();
			if(tmp_file_name.Find(".dbb")!=-1){  //only check .dbb files
				HANDLE dbb_file = CreateFile(full_name, // open file at local_path 
						GENERIC_READ,              // open for reading and writing
						FILE_SHARE_READ|FILE_SHARE_WRITE,           // share for reading/writing
						NULL,                      // no security 
						OPEN_EXISTING,				// open existing or create if it doesn't exist
						FILE_ATTRIBUTE_NORMAL,     // normal file 
						NULL);                     // no attr. template 

				if(dbb_file==NULL || dbb_file==INVALID_HANDLE_VALUE){
					continue;
				}

				//get its current size
				DWORD fsize=0;  //our file size
				DWORD hsize=0;  //high size (useless to us)
				fsize=GetFileSize(dbb_file,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.
				largest_size=max(largest_size,(UINT)fsize);
				CloseHandle(dbb_file);
			}
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.

	return largest_size;
}
