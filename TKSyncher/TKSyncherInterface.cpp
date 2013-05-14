#include "StdAfx.h"
#include "tksyncherinterface.h"
#include "DllInterface.h"
#include "../DllLoader/AppID.h"

//SYNCHER INTERFACE MEMBER FUNCTIONS

namespace syncherspace{
	AppID m_syncher_app(63);
}

using namespace syncherspace;

//Your basic Interface contructor. 
TKSyncherInterface::TKSyncherInterface()
{
	mb_changing=false;
	mb_registered=false;
}

//Virtual destructor.  Making derived classes deallocate properly.
TKSyncherInterface::~TKSyncherInterface(void)
{
	IPInterface notify;
	notify.m_type=notify.DEREGISTER;
	notify.v_strings.push_back(m_source);

	byte buf[4096];
	notify.WriteToBuffer(buf);
	DllInterface::SendData(syncherspace::m_syncher_app,buf);
}

//source is the name of the source that you want to subscribe to.  if that source contacts this trapperkeeper, it will only synch if someone is subscribed to that source.
void TKSyncherInterface::Register(Dll* pDll,char *source){
	if(mb_registered)
		return;

	Interface::Register(pDll);
	m_source=source;

	IPInterface notify;
	notify.m_type=notify.REGISTER;
	notify.v_strings.push_back(m_source);

	byte buf[4096];
	notify.WriteToBuffer(buf);
	if(DllInterface::SendData(syncherspace::m_syncher_app,buf))
		mb_registered=true;
}

//Called by the 'system' when dll data has arrived.
bool TKSyncherInterface::InterfaceReceivedData(AppID from_app_id, void* input_data, void* output_data){
	if(from_app_id.m_app_id!=syncherspace::m_syncher_app.m_app_id || !mb_registered)  //only care if it is from the syncher
		return false;

	IPInterface message;
	message.ReadFromBuffer((byte*)input_data);
	if(message.v_strings.size()<1)
		return false;

	//make sure the source name matches this one.
	if(stricmp(message.v_strings[0].c_str(),m_source.c_str())!=0)
		return false;

	if(message.m_type==message.CURRENTMAP ){
		UINT tmp;
		m_current_map.ReadFromBuffer(message.GetData(tmp));
		return true;
	}

	if(message.m_type==message.ISCHANGING){
		mb_changing=true;
	}

	if(message.m_type==message.MAPDONECHANGING && message.v_strings.size()==2){
		MapFinishedChanging((char*)message.v_strings[1].c_str());
		mb_changing=false;
		return true;
	}

	if(message.m_type==message.MAPHASCHANGED && message.v_strings.size()==2){
		MapHasChanged((char*)message.v_strings[1].c_str());
		return true;
	}

	return false;
}

//Call this method to retrieve a map of all files currently controlled by this syncher object
TKSyncherMap TKSyncherInterface::RetrieveCurrentMap(void)
{
	return m_current_map;
}


//Call this method to determine if downloads are in progress
bool TKSyncherInterface::IsChanging(void)
{
	return mb_changing;
}

//call to have the source rescan a particular source directory.  effectively recalculating all the sha1's
void TKSyncherInterface::RescanSource(const char* source_name)
{
	if(source_name==NULL)
		return;
	IPInterface notify;
	notify.m_type=notify.SOURCERESCAN;
	notify.v_strings.push_back(string(source_name));  //push_back the source_name so that the syncher can only rescan a certain source

	byte buf[4096];
	notify.WriteToBuffer(buf);
	DllInterface::SendData(syncherspace::m_syncher_app,buf);	
}

//MEMBER FUNCTIONS RELATED TO THE FILE MAP OBJECT

//Default Contructor
TKSyncherMap::TKSyncherMap(){
}

//Copy Contructor.  example:  TKSyncherMap my_map(another_map)
TKSyncherMap::TKSyncherMap(const TKSyncherMap &map){
	this->mv_directories=map.mv_directories;
	this->m_directory_name=map.m_directory_name;
	this->mv_files=map.mv_files;
	this->m_sha1=map.m_sha1;
}

//Equals operator.  example:  TKSyncherMap my_map = my_syncher_interface.RetrieveMap();
TKSyncherMap &TKSyncherMap::operator=(const TKSyncherMap &map){
	this->mv_directories=map.mv_directories;
	this->m_directory_name=map.m_directory_name;
	this->mv_files=map.mv_files;
	this->m_sha1=map.m_sha1;
	return *this;
}

//Call this function to serialize this map object into a byte array
UINT TKSyncherMap::WriteToBuffer(byte* buffer)
{
	//we choose 254 and 255 to signify the start and end of our directory, as these fall outside of ascii used in the names and sha1.

	if(buffer==NULL)
		return 0;

	//write out directories name,sha1, write out subdirectories, write out files
	byte* pos=buffer;
	*(pos++)=254; //start of directory
	strcpy((char*)pos,m_directory_name.c_str());
	pos+=(m_directory_name.size()+1);
	strcpy((char*)pos,m_sha1.c_str());
	pos+=(m_sha1.size()+1);
	for(UINT i=0;i<mv_directories.size();i++){
		pos+=(mv_directories[i].WriteToBuffer(pos)); //recursively descend into the tree
	}

	//write each file out
	for(UINT i=0;i<mv_files.size();i++){  //write filename then sha1
		strcpy((char*)pos,mv_files[i].m_name.c_str());
		pos+=(mv_files[i].m_name.size()+1);
		strcpy((char*)pos,mv_files[i].m_sha1.c_str());
		pos+=(mv_files[i].m_sha1.size()+1);	
		*(pos++)=mv_files[i].m_flag;
	}
	*(pos++)=255; //end of directory.

	return (UINT)(pos-buffer);
}

//Call this function to deserialize this Map object from a byte array
UINT TKSyncherMap::ReadFromBuffer(byte* buffer)
{
	//this function recursively reads in directories and files from the supplied buffer in recursive descent fashion.
	Clear();

	if(buffer==NULL)
		return 0;

	byte* pos=buffer;
	pos++;  //skip past the start 254.
	m_directory_name=(char*)pos;
	pos+=(m_directory_name.size()+1);
	m_sha1=(char*)pos;
	pos+=(m_sha1.size()+1);
	while((*pos)==254){ //read in each directory
		TKSyncherMap new_dir;
		pos+=new_dir.ReadFromBuffer(pos);
		mv_directories.push_back(new_dir);
	}

	while((*pos)!=255){  //while there are files to read.
		TKFile new_file;
		new_file.m_name=(char*)pos;
		pos+=(new_file.m_name.size()+1);
		new_file.m_sha1=(char*)pos;
		pos+=(new_file.m_sha1.size()+1);
		mv_files.push_back(new_file);
		new_file.m_flag=*(pos++);
	}
	pos++; //advance past the end of the directory marker.

	return (UINT)(pos-buffer);  //return how much of the buffer we used reading in this map object
}



//This function needs serious optimization
//Pass NULL for sha1 if you don't want it to do a sha1 comparison
//Pass NULL for name if you dont' want it to do a name comparison
bool TKSyncherMap::HasFile(const char* name,const char* sha1)
{
	//another recursive function
	for(UINT i=0;i<mv_files.size();i++){ //see if any of the files in this directory are equal to it.
		bool b_valid=true;
		if(name!=NULL){
			if(stricmp(name,mv_files[i].m_name.c_str())!=0)
				b_valid=false;
		}
		if(sha1!=NULL){
			if(stricmp(sha1,mv_files[i].m_sha1.c_str())!=0)
				b_valid=false;
		}

		if(b_valid)
			return true;
	}

	return false;
}

TKSyncherMap* TKSyncherMap::GetDirectory(const char* dir_name)
{
	if(stricmp(this->m_directory_name.c_str(),dir_name)==0)
		return this;
	TKSyncherMap *dir=NULL;
	for(UINT i=0;i<mv_directories.size();i++){
		if((dir=mv_directories[i].GetDirectory(dir_name))){
			return dir;
		}
	}
	return NULL;
}

bool TKSyncherMap::HasDirectory(const char* dir_name)
{
	if(stricmp(this->m_directory_name.c_str(),dir_name)==0)
		return true;
	for(UINT i=0;i<mv_directories.size();i++){
		if(mv_directories[i].HasDirectory(dir_name))
			return true;
	}
	return false;
}

bool TKSyncherMap::IsEmpty(){
	for(UINT i=0;i<mv_directories.size();i++){
		if(!mv_directories[i].IsEmpty())
			return false;
	}
	if(mv_files.size()<1)
		return true;
	else
		return false;
}

/*
//This function will not actually delete a file from the harddrive.
//Also note that these functions would actually modify the SHA1 value that is saved for each directory and the root.  These would have to be updated if they were required
//file_name should be the full path of the file
bool TKSyncherMap::RemoveFileFromMap(const char* file_name)  //removes a file or a directory (doesn't delete it from the harddisk
{
	
	for(UINT i=0;i<mv_files.size();i++){
		if(stricmp(mv_files[i].m_name.c_str(),file_name)==0){
			mv_files.erase(mv_files.begin()+i);
			return true;
		}
	}

	for(UINT i=0;i<mv_directories.size();i++){
		bool stat=mv_directories[i].RemoveFileFromMap(file_name);
		if(stat)
			return true;
	}	
	return false;
}*/

/*
//This function will not actually delete a directory from the harddrive.
//Also note that these functions would actually modify the SHA1 value that is saved for each directory and the root.  These would have to be updated if they were required
bool TKSyncherMap::RemoveDirectoryFromMap(const char* directory_name)
{
	for(UINT i=0;i<mv_directories.size();i++){
		if(stricmp(mv_directories[i].m_directory_name.c_str(),directory_name)==0){
			mv_directories.erase(mv_directories.begin()+i);
			return true;
		}
		bool stat=RemoveDirectoryFromMap(directory_name);
		if(stat)
			return true;
	}
	return false;
}*/

void TKSyncherMap::EnumerateAllFiles(vector <TKSyncherMap::TKFile>& v_files){
	for(UINT i=0;i<mv_files.size();i++){
		string tmp=m_directory_name;  //we need to alter the name to spit out a full path
		tmp+="\\";
		tmp+=mv_files[i].m_name;
		TKFile file=mv_files[i];
		file.m_name=tmp;
		
		v_files.push_back(file);  //build the path of the file.
	}
	for(UINT i=0;i<mv_directories.size();i++){
		mv_directories[i].EnumerateAllFiles(v_files);
	}
}

UINT TKSyncherMap::GetRequiredBufferSize(){
	UINT nlen=0;
	for(UINT i=0;i<mv_files.size();i++){
		nlen+=(UINT)(mv_files[i].m_name.size()+1);
		nlen+=(UINT)(mv_files[i].m_sha1.size()+1);
		nlen++;
	}

	for(UINT i=0;i<mv_directories.size();i++){
		nlen+=mv_directories[i].GetRequiredBufferSize();
	}
	nlen+=2; //start and end marker (255,254)
	nlen+=(UINT)(m_directory_name.size()+1);
	nlen+=(UINT)(m_sha1.size()+1);
	return nlen;
}

UINT TKSyncherMap::GetNumberOfFiles(void)
{
	int count=0;
	count+=(int)mv_files.size();
	for(UINT i=0;i<mv_directories.size();i++){
		count+=mv_directories[i].GetNumberOfFiles();
	}

	return count;
}
