#include "StdAfx.h"
#include "synchersubscriber.h"
#include "DllInterface.h"
#include "Source.h"
#include "sha1.h"
#include "SyncherDialog.h"

using namespace syncherspace;
#define FILECODE 1  //change this if the file format that this writes in changes.  This will prevent crashes when the syncher loads because its reading freaky data that it doesn't understand.

SyncherSubscriber::SyncherSubscriber(Dll *dll,const char* source,UINT app)
{
	p_dll=dll;
	m_source_name=source;
	mv_apps.push_back(app);
	p_file_transfer=new SyncherFileTransfer(this);
	p_file_transfer->Register(p_dll);
	m_current_transfers=0;
	LoadState();
	mb_valid=false;
	p_parent=NULL;
	mb_source_changed=false;
	m_last_server_ping=0;
}

SyncherSubscriber::~SyncherSubscriber(void)
{
	if(p_file_transfer) delete p_file_transfer;
}

void SyncherSubscriber::AddRef(UINT app)
{
	CSingleLock lock(&m_data_lock,TRUE);
	mv_apps.push_back(app);
}

//returns the number of apps still subscribed to this source
UINT SyncherSubscriber::DeRef(UINT app)
{
	CSingleLock lock(&m_data_lock,TRUE);

	for(UINT i=0;i<mv_apps.size();i++){
		if(mv_apps[i]==app){
			mv_apps.erase(mv_apps.begin()+i);
			return (UINT)mv_apps.size();
		}
	}
	return (UINT)mv_apps.size();
}

//Called by the file transfer interface indicating that we have received one of the files that this subscriber asked for.
void SyncherSubscriber::ReceivedFile(const char* local_path,const char* remote_path)
{
	CSingleLock lock(&m_data_lock,TRUE);
	TRACE("SYNCHER SERVICE:  SyncherSubscriber::ReceivedFile() %s.\n",local_path);
	m_current_transfers--;
	m_current_transfers=max(0,m_current_transfers);
	mv_received_jobs.Add(new ReceivedFileJob(local_path,remote_path));

	//remove this file from the list of files we think we are getting, it got added to a list that we know we have received, which will in turn be integrated into the current map.
	for(UINT i=0;i<mv_get_jobs.Size();i++){
		GetFileJob *job=(GetFileJob*)mv_get_jobs.Get(i);
		if(stricmp(job->m_local_path.c_str(),local_path)==0){
			mv_get_jobs.Remove(i);
			i--;
		}
	}
	Invalidate();
}

//we failed to download a file.  we will continue to try forever or until whoever has a file by the same name unlocks it, or the source no longer has it.
void SyncherSubscriber::FileDownloadFailed(const char* local_path)
{	
	TRACE("SYNCHER SERVICE:  SyncherSubscriber::FileDownloadFailed() %s.\n",local_path);
	CSingleLock lock(&this->m_data_lock,TRUE);
	m_current_transfers--;
	m_current_transfers=max(0,m_current_transfers);

	//remove it from the list of files that we think we are getting.
	for(UINT i=0;i<(UINT)mv_get_jobs.Size();i++){ 
		GetFileJob* job=(GetFileJob*)mv_get_jobs.Get(i);
		if(stricmp(job->m_local_path.c_str(),local_path)==0){
			mv_get_jobs.Remove(i);
			i--;
		}
	}
	Invalidate();
}


bool SyncherSubscriber::Validate(void)
{
	bool stat=false;

	//get failsafe added 6.11.03 to ensure that the get list never becomes corrupt, though it shouldn't anyways.  just fault tolerance.
	CSingleLock lock(&m_data_lock,TRUE);
	for(UINT i=0;i<(UINT)mv_get_jobs.Size();i++){
		GetFileJob* job=(GetFileJob*)mv_get_jobs.Get(i);
		//BUG:  Has crashed here before.  The lock above was added, this should fix it.
		if(job->m_created.HasTimedOut(60*60*24)){  //if this get file job has been here a long ass time, then for whatever reason, drop it.
			mv_get_jobs.Remove(i);
			i--;
			m_current_transfers--;
			m_current_transfers=max(0,m_current_transfers);
			mb_valid=false;
		}
	}


	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber *ss=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		if(ss->Validate())
			stat=true;
	}	
	if(mb_valid)
		return stat;
	if(m_source_ip.size()<1){  //Don't do anything until a source has contacted us.  We do have the last known current map available to any subscribers that wants it, all the files should be there unless they deleted them.
		return stat;
	}
	if(m_last_validate.HasTimedOut(2)){
		mb_valid=true;
		TRACE("TKSyncher SyncherSubscriber::Validate Source %s updating state.\n",m_source_name.c_str());
		UpdateState();
		m_last_validate.Refresh();
		return true;
	}
	else return stat;
	//ProcessDeleteJobs();
}

//check both the get file jobs, and the received file jobs incase it got here and the system just doesn't know it yet
bool SyncherSubscriber::IsGettingFile(const char* file)
{
	CSingleLock lock(&m_data_lock,TRUE);  //create a critical section as we should wait if an update is taking place
	for(UINT i=0;i<(UINT)mv_get_jobs.Size();i++){
		GetFileJob* job=(GetFileJob*)mv_get_jobs.Get(i);
		if(stricmp(job->m_local_path.c_str(),file)==0)
			return true;
	}

	for(UINT i=0;i<(UINT)mv_received_jobs.Size();i++){
		ReceivedFileJob* job=(ReceivedFileJob*)mv_received_jobs.Get(i);
		if(stricmp(job->m_local_path.c_str(),file)==0)
			return true;
	}
	return false;
}

void SyncherSubscriber::SetSourceMap(TKSyncherMap &map,const char *source_ip)
{
	//subscriber_children
	CSingleLock lock(&m_data_lock,TRUE);  //create a critical section as we should wait if an update is taking place
	
	m_source_map=map;
	ChangeNames(m_source_map);
	m_source_ip=source_ip;

	RemoveDebris(m_source_map.m_directory_name.c_str());
	TRACE("SYNCHER SERVICE:  The subscriber %s's map was set to a map having a sha1 of %s.\n",m_source_name.c_str(),map.m_sha1.c_str());
	Invalidate();
	mb_source_changed=true;
}

//this sends out a message to all subscribers that the map has changed
void SyncherSubscriber::UpdateSubscribers(void)
{
	if(p_parent){  //travel to the top and output that map to the subscribers.
		p_parent->UpdateSubscribers();
		return;
	}
	TKSyncherInterface::IPInterface message1;
	message1.m_type=message1.CURRENTMAP;
	message1.v_strings.push_back(this->m_source_name);

	//REMEMBER TO DELETE[] THIS
	TKSyncherMap tmap=m_current_map;
	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber* ss=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		tmap.mv_directories.push_back(ss->m_current_map);  //pretend they are directories, though they really are not.
	}
	UINT nrsize=tmap.GetRequiredBufferSize()+(1<<18);
	byte *data=new byte[nrsize];  //allocate a 2 meg piece of memory to write out the map.  This is to handle worst case scenarios.
	UINT size=tmap.WriteToBuffer(data);
	message1.SetData(data,size);
	UINT nw1=message1.WriteToBuffer(data);

	TKSyncherInterface::IPInterface message2;
	message2.m_type=message2.MAPHASCHANGED;
	message2.v_strings.push_back(m_source_name);
	message2.v_strings.push_back(m_source_ip);
	byte message_buf2[4096];
	UINT nw2=message2.WriteToBuffer(message_buf2);

	bool b_equal=false;

	//dispatch a 3rd message saying we are done
	TKSyncherInterface::IPInterface message3;
	byte message_buf3[4096];
	UINT nw3;

	if(IsEqual()){  //if we are now as equal to the source_map as we are going to get, then we are done changing
		TRACE("SYNCHER SERVICE:  Sending MAP FINISHED CHANGING notification to the appropriate clients.\n");
		b_equal=true;
		message3.m_type=message3.MAPDONECHANGING;
		message3.v_strings.push_back(m_source_name);
		message3.v_strings.push_back(m_source_ip);
		nw3=message3.WriteToBuffer(message_buf3);		
	}

	for(UINT i=0;i<mv_apps.size();i++){
		AppID app_id;
		app_id.m_app_id=mv_apps[i];
		g_syncher_dialog.Dispatch(app_id.m_app_id,data,nw1);
		//DllInterface::SendData(app_id,data);  //dispatch the current map to each of the subscribing plugins
	}

	for(UINT i=0;i<mv_apps.size();i++){
		AppID app_id;
		app_id.m_app_id=mv_apps[i];
		g_syncher_dialog.Dispatch(app_id.m_app_id,message_buf2,nw2);
		
		if(b_equal)
			g_syncher_dialog.Dispatch(app_id.m_app_id,message_buf3,nw3);  //dispatch a done message to each of the subscribing plugins
	}

	delete[] data;
	
}

void SyncherSubscriber::UpdateState()
{
	//Any one of the following situations may have occured to trigger this function being called
	//1.  A source has contacted this trapper keeper for the first time
	//2.  Files have been added to the source
	//2.  Files have been deleted from the source


	srand(timeGetTime());
	//TRACE("SYNCHER SERVICE:  SyncherSubscriber::UpdateState()  current file transfers = %d\n",m_current_transfers);
	//remove files that are mapped that are not on the source
	bool b_changed_state=false;
	if(mb_source_changed){
		b_changed_state=CleanMap(m_current_map);  //only clean the map if the source map has changed.  Cleaning the map removes files from the local map that are not on the source.
		mb_source_changed=false;

		vector <TKSyncherMap::TKFile> v_source_files;
		m_source_map.EnumerateAllFiles(v_source_files);
	}

	bool b_update_subscribers=b_changed_state; //also update subscribers if we deleted any files from the map

	//if we have received files, integrate them into the current map
	if(mv_received_jobs.Size()>0){
		b_changed_state=true;
		b_update_subscribers=true;
		//now, current map is a map of files currently on this host.


		//The most cpu intensive part of the syncher is right here.  It sucks but there is a lot of convenience to doing it this way.
		TKSyncherMap tmp_map=m_current_map;  //make a copy of the actual map we currently have
		m_current_map=m_source_map;  //lets assign our current_map to the source map.

		//clean out anything in the cur_map that we don't really have yet.
		UpdateStateEx(m_current_map,tmp_map);
		//update our sha for our current map.
		Source::CalculateDirectorySHA1(m_current_map);  //update the sha1s for each directory
		mv_received_jobs.Clear();
	}
	else if(stricmp(this->m_current_map.m_sha1.c_str(),this->m_source_map.m_sha1.c_str())!=0){  //if there are any differences, then we should still update the state.
		//in this situation we actually don't need to update the subscribers
		//request some files if necessary.

		//The most cpu intensive part of the syncher is right here.  It sucks but there is a lot of convenience to doing it this way.
		TKSyncherMap tmp_map=m_current_map;  //make a copy of the actual map we currently have
		m_current_map=m_source_map;  //lets assign our current_map to the source map.

		if(UpdateStateEx(m_current_map,tmp_map)){  //request any files if need be.
			b_changed_state=true;
			b_update_subscribers=true;
		}

		//update our sha for our current map.
		Source::CalculateDirectorySHA1(m_current_map);  //update the sha1s for each directory
	}

	//clear out any left over received files.  This shouldn't happen unless the source map was changed in between a call to get file and when the file finished downloading.  Also the source map would have to not have this file in it to have any left over received files.

	//if anything has changed, output our current state to a file.
	if(b_changed_state){
		if(p_parent)  //only have the parent Save its state if this is a child.
			p_parent->SaveState();
		else
			SaveState();
	}

	if(b_update_subscribers){ //if the map changed, then notify the subscribing plugins
		TRACE("SYNCHER SERVICE:  Sending MAP CHANGED notification to the appropriate clients.\n");
		if(p_parent)  //make sure only the topmost parent does the subscriber notifications.
			p_parent->UpdateSubscribers();
		else UpdateSubscribers();
		TRACE("SYNCHER SERVICE:  Done Sending MAP CHANGED notification to the appropriate clients.\n");
	}
}

//delete files that we don't actually have yet from the source
bool SyncherSubscriber::UpdateStateEx(TKSyncherMap &current_map,TKSyncherMap &actual_map)
{
	bool b_needs_save=false;
	bool b_maxed_gets=false;
	//for each file on the source
	TKSyncherMap *directory=actual_map.GetDirectory(current_map.m_directory_name.c_str());
	
	
	int start_index=rand()%max(1,(int)current_map.mv_files.size());  //we want to get random files so that the whole network isn't synching the same files at the same time.  it improves the performance of the p2p system

	//make a pass through to queue up some files
	for(UINT counter=0;counter<current_map.mv_files.size();counter++){
		int file_index=(counter+start_index)%current_map.mv_files.size();
		TKSyncherMap::TKFile *file=&current_map.mv_files[file_index];
		
		bool b_found=false;
		if(!directory || !directory->HasFile(file->m_name.c_str(),NULL)){ //check if we have this file on the host in our current_map
			string file_path=current_map.m_directory_name;
			file_path+="\\";
			file_path+=file->m_name.c_str();
			for(UINT j=0;j<mv_received_jobs.Size();j++){ //check if it is in the received file vector
				ReceivedFileJob *job=(ReceivedFileJob*)mv_received_jobs.Get(j);
				//const char *str1=job->m_local_path.c_str();
				//const char *str2=file_path.c_str();
				if(stricmp(job->m_local_path.c_str(),file_path.c_str())==0){  //it is in our received files
					//TRACE("Cancel Recieving File: %s\n",file_path.c_str());
					//mv_received_jobs.Remove(j);  //we can remove this file from the received files.
					b_found=true;
					break;
				}
			}
			if(!b_found){		
				bool can_get_more=false;
				if(!b_maxed_gets){
					can_get_more=GetCurrentTransferWeight()<MAXFILETRANSFERS;
					if(!can_get_more)
						b_maxed_gets=true;
				}
				if(!can_get_more)
					break;  //we are done requesting files for now
				if( can_get_more && !IsGettingFile(file_path.c_str())){
					//issue a command to acquire the file
					CString path=file_path.c_str();
					path.MakeLower();
					if(path.Find("c:\\syncher\\rcv\\media.distribute")!=-1 || path.Find("c:\\syncher\\rcv\\swarmer.distribute")!=-1){
						path.Replace("c:\\syncher\\rcv\\media.distribute","c:\\syncher\\file deposit\\distribute");
						path.Replace("c:\\syncher\\rcv\\swarmer.distribute","c:\\syncher\\file deposit\\distribute");
					}
					else{
						path.Replace("c:\\syncher\\rcv\\","c:\\syncher\\src\\");  //make sure we reference it relative to the source for a get
					}
					
					if(!DoesFileExistAlready(file_path.c_str(),file->m_sha1.c_str())){
						if(p_file_transfer->HasP2P()){
							p_file_transfer->P2PGetFile("p2p",file_path.c_str(),file->m_sha1.c_str());
							TRACE("SYNCHER SERVICE:  P2P Requesting file %s saving as %s.\n",file->m_sha1.c_str(),file_path.c_str());
						}
						else{
							p_file_transfer->GetFile(m_source_ip.c_str(),file_path.c_str(),(LPCSTR)path); //actually get the file if it doesn't exist on this client
							TRACE("SYNCHER SERVICE:  Requesting file %s from %s, saving as %s.\n",path,m_source_ip.c_str(),file->m_name.c_str());
						}
						m_current_transfers++;
						mv_get_jobs.Add(new GetFileJob(file_path.c_str()));
					}
					else{
						b_needs_save=true;
						//TRACE("Recieving File: %s\n",file_path.c_str());
						//ReceivedFile(file_path.c_str(),(LPCSTR)path);  //walla we have 'received' this file, of course its already here, but lets just reuse code we already have.
						continue;  //we will pretend we have received this file.
					}
				}
				//remove it from our current map cus it isn't here yet.
				//current_map.mv_files.erase(current_map.mv_files.begin()+file_index);  //we just don't have this file yet.
				//counter--;
			}
		}
	}


	
	//make a pass through to clear out the files we don't have yet
	for(UINT counter=0;counter<current_map.mv_files.size();counter++){
		TKSyncherMap::TKFile *file=&current_map.mv_files[counter];
		
		bool b_found=false;
		if(!directory || !directory->HasFile(file->m_name.c_str(),NULL)){ //check if we have this file on the host in our current_map
			string file_path=current_map.m_directory_name;
			file_path+="\\";
			file_path+=file->m_name.c_str();
			for(UINT j=0;j<mv_received_jobs.Size();j++){//check if it is in the received file vector
				ReceivedFileJob *job=(ReceivedFileJob*)mv_received_jobs.Get(j);
				//const char *str1=job->m_local_path.c_str();
				//const char *str2=file_path.c_str();
				if(stricmp(job->m_local_path.c_str(),file_path.c_str())==0){  //it is in our received files
					//TRACE("Cancel Recieving File: %s\n",file_path.c_str());
					mv_received_jobs.Remove(j);  //we can remove this file from the received files.
					b_found=true;
					break;
				}
			}
			if(!b_found && !DoesFileExistAlready(file_path.c_str(),file->m_sha1.c_str())){		
				current_map.mv_files.erase(current_map.mv_files.begin()+counter);  //we just don't have this file yet.
				counter--;
			}
		}
	}
	
	int rand_directory_offset=rand()%max(1,(int)current_map.mv_directories.size());
	//iterate through the subdirectories, but starting at a random directory offset.  so each rack is getting different files, and utilizing p2p action more.	
	for(UINT i=0;i<current_map.mv_directories.size();i++){
		if(UpdateStateEx(current_map.mv_directories[(i+rand_directory_offset)%current_map.mv_directories.size()],actual_map))
			b_needs_save=true;
	}

	return b_needs_save;
}


bool SyncherSubscriber::CleanMap(TKSyncherMap& cur_map)
{

	bool stat=false;

	//go through the files in this directory
	TKSyncherMap* p_directory=m_source_map.GetDirectory(cur_map.m_directory_name.c_str());
	
	if(!p_directory){
		if(cur_map.mv_directories.size()>0 || cur_map.mv_files.size()>0)
			stat=true;
		cur_map.Clear();
	}

	//clean out unnecessary empty directories
	for(UINT i=0;i<cur_map.mv_directories.size();i++){
		if(!p_directory->HasDirectory(cur_map.mv_directories[i].m_directory_name.c_str())){
			cur_map.mv_directories.erase(cur_map.mv_directories.begin()+i);
			stat=true;
			i--;
		}
		else{
			if(CleanMap(cur_map.mv_directories[i]))
				stat=true;
		}
	}

	for(UINT j=0;j<cur_map.mv_files.size();j++){
		//const char *name=cur_map.mv_files[j].m_name.c_str(); //tmp, delete me
		if(!p_directory->HasFile(cur_map.mv_files[j].m_name.c_str(),cur_map.mv_files[j].m_sha1.c_str())){
			stat=true;
			cur_map.mv_files.erase(cur_map.mv_files.begin()+j);
			j--;  //remove this file.
		}
	}

	return stat;
}

void SyncherSubscriber::ChangeNames(TKSyncherMap &map)
{
	CString tmp=map.m_directory_name.c_str();
	tmp=tmp.MakeLower();
	tmp.Replace("c:\\syncher\\src\\","c:\\syncher\\rcv\\");
	if(stricmp(m_source_name.c_str(),"media.distribute")==0){
		tmp.Replace("c:\\syncher\\file deposit\\distribute","c:\\syncher\\rcv\\Media.distribute");
	}
	else if(stricmp(m_source_name.c_str(),"swarmer.distribute")==0){
		tmp.Replace("c:\\syncher\\file deposit\\distribute","c:\\syncher\\rcv\\Swarmer.distribute");
	}
	map.m_directory_name=(LPCSTR)tmp;
	//change each of the files names
	for(UINT i=0;i<map.mv_files.size();i++){
		tmp=map.mv_files[i].m_name.c_str();
		//tmp.Replace("c:\\syncher\\src\\","c:\\syncher\\rcv\\");
		int index=tmp.ReverseFind('\\');
		if(index==-1){
			continue;
		}
		tmp=tmp.Mid(index+1);
		map.mv_files[i].m_name=(LPCSTR)tmp;
	}
	//do all that again for each of the sub directories.
	for(UINT i=0;i<map.mv_directories.size();i++){
		ChangeNames(map.mv_directories[i]);
	}
}

/*
void SyncherSubscriber::ProcessDeleteJobs(void)
{
	for(UINT i=0;i<(UINT)mv_delete_jobs.Size();i++){
		DeleteFileJob* job=(DeleteFileJob*)mv_delete_jobs.Get(i);
		//if(m_current_map

		//Check if the current_map has changed its mind about this.
		if(job->mb_directory){
			if(m_current_map.HasDirectory(job->m_local_path.c_str())){
				TRACE("Skipping deletion of directory %s.\n",job->m_local_path.c_str());
				mv_delete_jobs.Remove(i);
				i--;
				continue;
			}
		}
		else if(!job->mb_directory && (m_current_map.HasFile(job->m_local_path.c_str(),NULL) || IsGettingFile(job->m_local_path.c_str()))){
			TRACE("Skipping deletion of file %s.\n",job->m_local_path.c_str());
			mv_delete_jobs.Remove(i);
			i--;
			continue;
		}

		//check if the file even exists
		OFSTRUCT tstruct;
		if(!job->mb_directory && (HFILE_ERROR==OpenFile( job->m_local_path.c_str(),   &tstruct,  	OF_EXIST	))){
			//this file doesn't exist anyways.
			mv_delete_jobs.Remove(i);
			i--;
			continue;
		}
		if(job->mb_directory){
			if(RemoveDirectory(job->m_local_path.c_str()) || GetLastError()==3){  //do the GetLastError check to see if the directory exists
				TRACE("SYNCHER SERVICE:  Subscriber source %s succesfully deleted the directory %s.\n",m_source_name.c_str(),job->m_local_path.c_str());
				mv_delete_jobs.Remove(i);  //if we successfully deleted it, we can remove it from our vector
				i--;
				continue;
			}
		}		
		else if(DeleteFile(job->m_local_path.c_str())){
			TRACE("SYNCHER SERVICE:  Subscriber source %s succesfully deleted the file %s.\n",m_source_name.c_str(),job->m_local_path.c_str());
			mv_delete_jobs.Remove(i);  //if we successfully deleted it, we can remove it from our vector
			i--;
			continue;
		}
	}
}
*/

//This function loads the state of all the maps and the vector of files that still need to be deleted.
//In order to protect the system in the event that TrapperKeeper crashes while the state is saving, a special byte is encoded on the end and it had better be there when it is read back in.
void SyncherSubscriber::LoadState(void)
{
	mv_subscriber_children.Clear();
	//specify the file which we wish to load from
	string file_name="c:\\syncher\\rcv_";
	file_name+=m_source_name;
	file_name+=".dat";
	HANDLE m_file = CreateFile(file_name.c_str(), // open file at local_path 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	if(m_file==INVALID_HANDLE_VALUE || m_file==NULL){  //this should always return null for all children.
		return;
	}

	DWORD hsize=0;
	DWORD fsize=GetFileSize(m_file,&hsize);

	byte *buffer=new byte[fsize+1];  //an adequate buffer to read in everything.
	DWORD bytes_read=0;
	
	try{
		ReadFile(m_file,buffer,fsize+1,&bytes_read,NULL);
		if(bytes_read<8){
			CloseHandle(m_file);
			delete [] buffer;
			return;
		}
		else{
			if(buffer[bytes_read-1]!=FILECODE){  //end of buffer validity check
				TRACE("SYNCHER SERVICE:  SyncherSubscriber::LoadState()  A subscriber source file is corrupt.  TrapperKeeper probably crashed during a SaveState() (please note it didn't crash because of SaveState()).");
				CloseHandle(m_file);
				delete [] buffer;
				return;
			}
			
			byte *pos=buffer;
			pos+=m_current_map.ReadFromBuffer(pos);
			pos++;
			pos+=m_source_map.ReadFromBuffer(pos);
			pos++;

			UINT child_count=*((UINT*)(pos));
			pos+=4;

			//read in each of the children
			for(UINT i=0;i<child_count;i++){
				string name=(char*)pos;
				pos+=name.size();
				pos++; //past the null char that signifies the end of the string
				SyncherSubscriber *ss=new SyncherSubscriber(p_dll,name.c_str(),mv_apps[0]);
				ss->SetParent(this);
				pos+=ss->m_current_map.ReadFromBuffer(pos);
				pos++;
				pos+=ss->m_source_map.ReadFromBuffer(pos);
				pos++;
				mv_subscriber_children.Add(ss);
			}
		}
	}
	catch(char* str){  
		char *ptr=str;  //gets rid of stupid unreferenced compiler warning in release versions.
		TRACE("SYNCHER SERVICE: Exception %s while calling SyncherSubscriber::LoadState().  This is very wierd.\n",str);
		m_source_map.Clear();  //fix any corruptedness.
		m_current_map.Clear();
	}

	CloseHandle(m_file);
	delete[] buffer;
}

//This function saves the state of all the maps and the vector of files that still need to be deleted.
//In order to protect the system in the event that TrapperKeeper crashes while the state is saving, a special byte is encoded on the end and it had better be there when it is read back in.
void SyncherSubscriber::SaveState(void)
{
	UINT bytes_written;
	UINT output_size=0;

	//specify the file that we want to save our state to.  This is derived from the source name.
	string file_name="c:\\syncher\\rcv_";
	file_name+=m_source_name;
	file_name+=".dat";

	HANDLE m_file = CreateFile(file_name.c_str(), // open file at local_path 
                GENERIC_WRITE,              // open for writing 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                CREATE_ALWAYS,             // create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	if(m_file==INVALID_HANDLE_VALUE || m_file==NULL){
		TRACE("SYNCHER SERVICE:  Couldn't save the state of an interface source.\n");
		return;
	}

	UINT total_size=(1<<19);
	total_size+=m_current_map.GetRequiredBufferSize();
	total_size+=m_source_map.GetRequiredBufferSize();

	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber *ss=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		total_size+=ss->m_current_map.GetRequiredBufferSize();
		total_size+=ss->m_source_map.GetRequiredBufferSize();
	}

	byte *buffer=new byte[total_size]; //a sufficient buffer to read in each of the maps.
	
	//write the current map out
	output_size+=m_current_map.WriteToBuffer(buffer+output_size);
	buffer[output_size++]=(byte)0;
	output_size+=m_source_map.WriteToBuffer(buffer+output_size);
	buffer[output_size++]=(byte)0;
	*((UINT*)(buffer+output_size))=mv_subscriber_children.Size();
	output_size+=4;
	//write out each of the children
	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber *ss=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		strcpy((char*)(buffer+output_size),ss->m_source_name.c_str());
		output_size+=(UINT)(ss->m_source_name.size()+1);
		output_size+=ss->m_current_map.WriteToBuffer(buffer+output_size);
		buffer[output_size++]=(byte)0;
		output_size+=ss->m_source_map.WriteToBuffer(buffer+output_size);
		buffer[output_size++]=(byte)0;
	}

	buffer[output_size++]=(byte)0;  //pad an extra 0 byte on the end

	bytes_written=0;
	for(int i=0;i<20 && bytes_written<output_size;i++){ //try for a moment to write all this data out.  
		int old_num=bytes_written;
		WriteFile(m_file,buffer+bytes_written,output_size-bytes_written,(DWORD*)&bytes_written,NULL);
		bytes_written=old_num+bytes_written;
		if(bytes_written<output_size)
			Sleep(10);
	}

	buffer[0]=FILECODE; //our end of buffer signal.
	WriteFile(m_file,buffer,1,(DWORD*)&bytes_written,NULL);  //write out a signal on the end of the stream.  This had better be there when we start back up or trapper keeper crashed while a call to save state is taking place.

	CloseHandle(m_file);
	delete [] buffer;
}

void SyncherSubscriber::Resynch(void)
{
	CSingleLock lock(&this->m_data_lock,TRUE);

	//have each of the source children clear out their files
	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber *ss=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		ss->m_current_map.Clear();
		ss->m_source_map.Clear();
		RemoveDebris(ss->m_current_map.m_directory_name.c_str());
	}	
	mv_subscriber_children.Clear();
	m_current_map.Clear();
	m_source_map.Clear();
	m_source_ip.clear();
	RemoveDebris(m_current_map.m_directory_name.c_str());
	
	SaveState();
}

/*
void SyncherSubscriber::DeleteFiles(TKSyncherMap &map)
{
	for(UINT i=0;i<map.mv_files.size();i++){
		TRACE("scheduling file %s for deletion.\n",map.mv_files[i].m_name.c_str());
		mv_delete_jobs.Add(new DeleteFileJob(map.mv_files[i].m_name.c_str(),false));
	}

	for(UINT i=0;i<map.mv_directories.size();i++){
		DeleteFiles(map.mv_directories[i]);
	}

	TRACE("scheduling directory %s for deletion.\n",map.m_directory_name.c_str());
	mv_delete_jobs.Add(new DeleteFileJob(map.m_directory_name.c_str(),true));
}*/

bool SyncherSubscriber::DoesFileExistAlready(const char* local_path, const char* _sha1)
{
	//does it even exist on the harddrive?
	OFSTRUCT tstruct;
	if(HFILE_ERROR==OpenFile( local_path,   &tstruct,  	OF_EXIST	)){
		return false;
	}

	//now we need to compare sha1s
	SHA1 sha1;
	sha1.HashFile((char*)local_path);
	sha1.Final();
	char existing_sha1[33];
	sha1.Base32Encode(existing_sha1);
	if(stricmp(_sha1,existing_sha1)==0){
		return true;
	}
	else
		return false;
}

//this function recursively deletes files and directories that are a children of the parameter 'directory'
//it will delete anything that isn't on the source.
void SyncherSubscriber::RemoveDebris(const char* directory)
{
	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	
	//make damn sure we are working in the syncher's rcv directory.
	if(path.Find("c:\\syncher\\rcv")==-1)  
		return;
	WIN32_FIND_DATA info;
	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	TKSyncherMap *tmp_map=m_source_map.GetDirectory(directory);

	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		bool b_directory=false;
		CString full_name=path+info.cFileName;
		if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
			b_directory=true;
			RemoveDebris(full_name);  //if it is a directory descend into it and clean it out.
		}

		if(b_directory){
			if(!tmp_map || !tmp_map->HasDirectory(full_name)){
				RemoveDirectory(full_name);
			}
		}
		else{
			if(!tmp_map || !tmp_map->HasFile(info.cFileName,NULL)){  //check by filename and not entire path
				DeleteFile(full_name);
			}
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
}

//invalidates this subscriber and any parents.
void SyncherSubscriber::Invalidate(void)
{
	mb_valid=false;
	if(p_parent)
		p_parent->Invalidate();
}

void SyncherSubscriber::SetParent(SyncherSubscriber* parent)
{
	p_parent=parent;
}

//returns NULL if this and none of this's children are the source
//returns this or the child that is the source
//NOTE:  This function will create a child of the specified source_name if it does't exist and it looks like it belongs to this subscriber
SyncherSubscriber* SyncherSubscriber::IsSource(const char* source_name)
{
	CSingleLock lock(&m_data_lock,TRUE);
	if(stricmp(source_name,m_source_name.c_str())==0)
		return this;

	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber *ss=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		if(ss->IsSource(source_name))
			return ss;
	}

	if(p_parent!=NULL)  //return if this is a child.  this tree has a depth of only 2
		return NULL;
	
	char* first_occurence=strchr(source_name,'.');

	//if it doesn't have a '.' then it can't possibly be a child
	if(first_occurence==NULL)
		return NULL;

	//if they don't have the same primary source (str before the '.'), then it isn't supposed to be a child
	if(strnicmp(m_source_name.c_str(),source_name,size_t(first_occurence-source_name))!=0)
		return NULL;

	SyncherSubscriber *ns=new SyncherSubscriber(p_dll,source_name,mv_apps[0]);
	ns->SetParent(this);
	mv_subscriber_children.Add(ns);
	return ns;
}

bool SyncherSubscriber::IsEqual(void)
{
	for(UINT i=0;i<m_current_map.mv_files.size();i++){
		const char *file_name=m_current_map.mv_files[i].m_name.c_str();
		CString tmp=file_name;
	}
	if(!m_current_map.IsEqual(m_source_map)){
		return false;
	}

	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber *ss=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		if(!ss->IsEqual())
			return false;
	}	

	return true;
}

void SyncherSubscriber::FillTreeCtrl(CTreeCtrl& tree,HTREEITEM *parent)
{
	CSingleLock lock(&m_data_lock,TRUE);
	HTREEITEM tree_item;
	HTREEITEM top_node;
	unsigned long cur_time=timeGetTime();
	if(cur_time<m_last_server_ping)
		m_last_server_ping=0;
	unsigned long secs=(cur_time-m_last_server_ping)/1000UL;
	if(parent==NULL){
		CString tmp;
		if(secs>10000){
			tmp.Format("%s  ::  %d total file transfers  ::  Source IP: %s, No source server contact yet, boooo",m_source_name.c_str(),this->CountTotalGets(),m_source_ip.c_str(),secs);
		}
		else{
			tmp.Format("%s  ::  %d total file transfers  ::  SourceIP: %s, pinged %d seconds ago",m_source_name.c_str(),this->CountTotalGets(),m_source_ip.c_str(),secs);
		}
		top_node=tree.InsertItem((LPCSTR)tmp,0,0);
	}
	else{
		CString tmp;
		if(secs>10000){
			tmp.Format("%s  ::  %d total file transfers  ::  Source IP: %s, No source server contact yet, boooo",m_source_name.c_str(),this->CountTotalGets(),m_source_ip.c_str(),secs);
		}
		else{
			tmp.Format("%s  ::  %d total file transfers  ::  SourceIP: %s, pinged %d seconds ago",m_source_name.c_str(),this->CountTotalGets(),m_source_ip.c_str(),secs);
		}
		top_node=tree.InsertItem((LPCSTR)tmp,0,0,*parent);
	}

	tree_item=tree.InsertItem("multisources",0,0,top_node);
	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber *subscriber=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		subscriber->FillTreeCtrl(tree,&tree_item);
	}

	tree_item=tree.InsertItem("Getting Files",0,0,top_node);
	vector <string> v_str_files;
	EnumGetFiles(v_str_files);
	for(UINT i=0;i<v_str_files.size();i++){
		CString tmp=v_str_files[i].c_str();
		tmp.Replace("c:\\syncher\\rcv","");
		tmp.Replace("c:\\syncher\\src","");
		tree.InsertItem(tmp,0,0,tree_item);
	}

	CString tstr;
	
	vector <TKSyncherMap::TKFile> v_source_files;
	m_source_map.EnumerateAllFiles(v_source_files);
	tstr.Format("Source Files (%d)",v_source_files.size());
	tree_item=tree.InsertItem(tstr,0,0,top_node);
	for(UINT i=0;i<v_source_files.size();i++){
		CString tmp=v_source_files[i].m_name.c_str();
		tmp.Replace("c:\\syncher\\rcv","");
		tmp.Replace("c:\\syncher\\src","");
		CString new_node_str;
		new_node_str.Format("%s    sha1=%s",tmp,v_source_files[i].m_sha1.c_str());
		tree.InsertItem((LPCSTR)new_node_str,0,0,tree_item);
	}

	
	
	vector <TKSyncherMap::TKFile> v_my_files;
	m_current_map.EnumerateAllFiles(v_my_files);
	tstr.Format("My Files (%d)",v_my_files.size());
	tree_item=tree.InsertItem(tstr,0,0,top_node);
	for(UINT i=0;i<v_my_files.size();i++){
		CString tmp=v_my_files[i].m_name.c_str();
		tmp.Replace("c:\\syncher\\rcv","");
		tmp.Replace("c:\\syncher\\src","");
		CString new_node_str;
		new_node_str.Format("%s    sha1=%s",tmp,v_my_files[i].m_sha1.c_str());
		tree.InsertItem((LPCSTR)new_node_str,0,0,tree_item);
	}

	tree_item=tree.InsertItem("Differences",0,0,top_node);
	for(UINT i=0;i<v_source_files.size();i++){
		const char* src_file=v_source_files[i].m_name.c_str();
		const char* src_sha=v_source_files[i].m_sha1.c_str();
		bool b_found=false;
		for(UINT j=0;j<v_my_files.size();j++){
			if(stricmp(v_my_files[j].m_name.c_str(),src_file)==0 && stricmp(v_my_files[j].m_sha1.c_str(),src_sha)==0){
				b_found=true;
				break;
			}
		}

		if(!b_found){
			CString new_node_str;
			new_node_str.Format("%s    sha1=%s",src_file,src_sha);
			tree.InsertItem((LPCSTR)new_node_str,0,0,tree_item);
		}
	}
}

void SyncherSubscriber::EnumGetFiles(vector <string> &files)
{
	CSingleLock lock(&m_data_lock,TRUE);  //create a critical section as we should wait if an update is taking place
	for(UINT i=0;i<(UINT)mv_get_jobs.Size();i++){
		GetFileJob* job=(GetFileJob*)mv_get_jobs.Get(i);
		files.push_back(job->m_local_path);
	}
}

UINT SyncherSubscriber::CountTotalGets(void)
{
	CSingleLock lock(&m_data_lock,TRUE);  //create a critical section as we should wait if an update is taking place
	UINT sum=mv_get_jobs.Size();
	
	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber *subscriber=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		sum+=subscriber->mv_get_jobs.Size();
	}

	return sum;
}

SyncherSubscriber* SyncherSubscriber::GetParent(void)
{
	return p_parent;
}

void SyncherSubscriber::PurgeChild(SyncherSubscriber* p_child,bool b_notify)
{
	CSingleLock lock(&m_data_lock,TRUE);  //create a critical section as we should wait if an update is taking place
	for(UINT i=0;i<mv_subscriber_children.Size();i++){
		SyncherSubscriber *subscriber=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		if(subscriber==p_child){
			subscriber->m_source_map.Clear();  //remove any known source files
			subscriber->RemoveDebris(subscriber->m_current_map.m_directory_name.c_str());  //remove all of those files
			RemoveDirectory(subscriber->m_current_map.m_directory_name.c_str());
			subscriber->m_current_map.Clear();
			mv_subscriber_children.Remove(i);  //delete this child
			i--;
			SaveState();  //save our state without this child in it.
			if(b_notify){
				UpdateSubscribers();
			}
			this->mb_valid=false;  //let the next update take care of all the synchronization
			return;
		}
	}
}

void SyncherSubscriber::PurgeAll(void)
{
	CSingleLock lock(&m_data_lock,TRUE);  //create a critical section as we should wait if an update is taking place
	m_source_map.Clear();  //remove any known source files
	RemoveDebris(m_current_map.m_directory_name.c_str());  //remove all of those files
	RemoveDirectory(m_current_map.m_directory_name.c_str());  //even remove the directory (which has to be empty for this call to succeed)
	m_current_map.Clear();
	for(UINT i=0;i<mv_subscriber_children.Size();i++){  //purge each of the children (multisources) too
		SyncherSubscriber *subscriber=(SyncherSubscriber*)mv_subscriber_children.Get(i);
		UINT old_size=mv_subscriber_children.Size();
		PurgeChild(subscriber,false);
		UINT new_size=mv_subscriber_children.Size();
		if(old_size!=new_size)  //i should hope it removed it, but lets not have anything crashing.
			i--;
	}
	SaveState();
	UpdateSubscribers();
	mb_valid=false;
}

int SyncherSubscriber::GetCurrentTransferWeight(void)
{
	float weight=0;
	for(UINT i=0;i<(UINT)mv_get_jobs.Size();i++){
		GetFileJob* job=(GetFileJob*)mv_get_jobs.Get(i);
		CString tmp=job->m_local_path.c_str();
		tmp=tmp.MakeLower();
		if(tmp.Find(".mp3")!=-1){
			weight+=.15f;
		}
		else if(tmp.Find(".avi")!=-1 || tmp.Find(".mpg")!=-1 || tmp.Find(".mpeg")!=-1 || tmp.Find(".asf")!=-1){
			weight+=1.5f;
		}
		else{
			weight+=1;
		}
	}
	return (int)weight;
}
