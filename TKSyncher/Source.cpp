#include "StdAfx.h"
#include "source.h"
#include "..\tkcom\SHA1.h"

Source::Source(char* name,char *dir)
{
	m_name=name;
	m_directory_name=dir;
	mb_delete=false;
	mb_updated=false;
	mb_updating=false;
	mb_cancel_update=0;
	LoadState();  //do we have a previous state saved for this here beast?
}

Source::~Source(void)
{
}

//call to have the source rescan its directories and update its sha1's.  Returns true if something has changed, false otherwise.
bool Source::Update(void)
{
	CSingleLock lock(&this->m_source_lock,TRUE);
	this->mb_updating=true;
	TKSyncherMap old_map=*this; //save our current state, so we can clear it and compare to it later.
	string old_sha1=this->m_sha1;
	string tmp=this->m_directory_name;
	this->Clear();		//remove everything from this source
	this->m_directory_name=tmp; 
	this->UpdateEngine(*this);  //add all the files and directories
	CalculateDirectorySHA1(*this); //update directory sha1's based on file sha1's
	TRACE("TKSyncher Source::Update() Finished updating source %s (all files have been scanned).\n",m_name.c_str());
	mb_updated=true;
	mb_updating=false;
	if(stricmp(old_sha1.c_str(),this->m_sha1.c_str())==0)  //compare the before and after sha1s
		return false; //if they are equal, then nothing changed
	else return true; //if not equal, then something changed
}

//called by Update and AddElement to build this source.
void Source::UpdateEngine(TKSyncherMap &map)
{
	//There is a recursive interaction between this function and AddElement ( they call each other over and over until the have enumerated every element in this directory, and each of its subdirectories)

	WIN32_FIND_DATA info;

	CString tmp=map.m_directory_name.c_str();
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	CString name;//=path+info.cFileName;
	//AddElement(map,(char*)(LPCSTR)name); //add the first element
	while(FindNextFile(hFind,&info) && !mb_cancel_update){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		name=path+info.cFileName;
		AddElement(map,name.GetBuffer(name.GetLength()),info.cFileName);
	}
	FindClose(hFind);
	
}

//called from UpdateEngine to recursively add new elements to this source
void Source::AddElement(TKSyncherMap& map, const char *full_file_name,const char *single_name)
{
	if(stricmp(single_name,".")==0 || stricmp(single_name,"..")==0)
		return;
	if(((GetFileAttributes(full_file_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){  //is it a directory?
		TKSyncherMap new_dir;
		new_dir.m_directory_name=full_file_name;
		UpdateEngine(new_dir); //add all the subdirectories under this element to it.
		map.mv_directories.push_back(new_dir); //add this new element with its subdirectories to the owner.
	}
	else{ //it is a file
		TKSyncherMap::TKFile file;
		file.m_name=single_name;
		//Sleep(5);  //take a break so other programs can do some work.
		
		if(CalculateFileSHA1(full_file_name,file.m_sha1)){  //here is where the hard work of maintaining this source is done.
			TRACE("TKSyncher Source::AddElement() Source with name %s added file named %s with sha1 of %s.\n",m_name.c_str(),full_file_name,file.m_sha1.c_str());
			map.mv_files.push_back(file);	
		}
		else{
			TRACE("TKSyncher Source::AddElement() Source with name %s FAILED add file named %s because it couldn't calculate the sha.\n",m_name.c_str(),full_file_name);
		}
	}
}

//call to recursively update the directory sha1's based on file sha1's
string Source::CalculateDirectorySHA1(TKSyncherMap &dir){
	SHA1 sha1;

	for(UINT i=0;i<dir.mv_directories.size();i++){
		if(!dir.mv_directories[i].IsEmpty()){  //lets not include empty subdirectories in our sha1.
			string sd_sha1=CalculateDirectorySHA1(dir.mv_directories[i]);
			sha1.Update((byte*)sd_sha1.c_str(),(UINT)sd_sha1.size());
		}
	}

	for(UINT i=0;i<dir.mv_files.size();i++){
		sha1.Update((byte*)dir.mv_files[i].m_sha1.c_str(),(UINT)dir.mv_files[i].m_sha1.size());
	}

	sha1.Final();  //we are done adding bytes to be included in the sha1
	char ba[33];
	ba[0]='\0';
	sha1.Base32Encode(ba);  //encode it in base32,  this is unnecessary, the 20 byte value would be more efficient, but we use this for readability and reference

	dir.m_sha1=ba;  
	return dir.m_sha1;
}

bool Source::CalculateFileSHA1(const char* file_name,string &hash)
{
	char ba[33];
	ba[0]='\0';
	SHA1 sha1;
	if(sha1.HashFile((char*)file_name,0,mb_cancel_update)){
		sha1.Final();
		sha1.Base32Encode(ba);
	}
	else
		return false;

	hash=string(ba);
	return true;
}

void Source::DeleteFilesAndDirectories(TKSyncherMap &map)
{ //recursively deletes all files and directories represented by the object passed in
	for(UINT i=0;i<map.mv_files.size();i++){
		DeleteFile(map.mv_files[i].m_name.c_str());
	}
	for(UINT i=0;i<map.mv_directories.size();i++){
		DeleteFilesAndDirectories(map.mv_directories[i]);
	}
	
	BOOL stat=RemoveDirectory(map.m_directory_name.c_str());
	if(!stat){
		//hrm wtf to do about this
	}
}

bool Source::HasBeenUpdatedOnce(void)
{
	return mb_updated;
}

void Source::Die(void)
{
	mb_delete=true;
}

bool Source::IsValid(void)
{
	return (!mb_delete && mb_updated);
}

bool Source::IsDestroyed(void)
{
	return mb_delete;
}

//Call to save the broadcast destinations for this source.  It is called by the dialog when a change is actually made.
void Source::SaveState(void)
{
	UINT bytes_written;
	UINT output_size;
	
	string file_name="c:\\syncher\\src_";
	file_name+=m_name;
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

	byte *buffer=new byte[1<<20];  //ample room to write out all the destinations
	byte *pos=buffer;
	
	for(UINT i=0;i<mv_destinations.size();i++){
		strcpy((char*)pos,mv_destinations[i].c_str());
		pos+=(mv_destinations[i].size()+1);
	}
	*(pos++)=253;

	//write the source map out
	output_size=(UINT)(pos-buffer);
	bytes_written=0;
	for(int i=0;i<20 && bytes_written<output_size;i++){ //try for a moment to write all this data out.  
		int old_num=bytes_written;
		WriteFile(m_file,buffer+bytes_written,output_size-bytes_written,(DWORD*)&bytes_written,NULL);
		bytes_written=old_num+bytes_written;
		if(bytes_written<output_size)
			Sleep(10);
	}

	CloseHandle(m_file);

	delete [] buffer;
}

//Call to load the broadcast definitions for this source.  Only done in the constructor.
void Source::LoadState(void)
{
	mv_destinations.clear();
	string file_name="c:\\syncher\\src_";
	file_name+=m_name;
	file_name+=".dat";
	HANDLE m_file = CreateFile(file_name.c_str(), // open file at local_path 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

	if(m_file==INVALID_HANDLE_VALUE || m_file==NULL){
		return;
	}

	byte *buffer=new byte[(1<<20)];  //a 1 meg chunk to read in everything at worst case
	DWORD bytes_read=0;
	
	try{
		ReadFile(m_file,buffer,1<<20,&bytes_read,NULL);
		if(bytes_read<1){
			CloseHandle(m_file);
			delete [] buffer;
			return;
		}
		else{
			if(buffer[bytes_read-1]!=253){  //end of buffer validity check
				TRACE("SYNCHER SERVICE:  Source::LoadState()  A source destination file is corrupt (end key was %d).  TrapperKeeper probably crashed during a SaveState() (please note it didn't crash because of SaveState()).",buffer[bytes_read-1]);
				CloseHandle(m_file);
				delete [] buffer;
				return;
			}
			byte *pos=buffer;
				
			while(((DWORD)(pos-buffer))<(bytes_read-1)){
				//read in the destinations
				string tmp=(const char*)pos;
				mv_destinations.push_back(tmp);
				pos+=(tmp.size()+1);
			}
		}
	}
	catch(char* str){  
		char *ptr=str;  //gets rid of stupid unreferenced compiler warning in release versions.
		TRACE("SYNCHER SERVICE: Exception %s while calling Source::LoadState().  This is very wierd.\n",str);
	}

	CloseHandle(m_file);
	delete[] buffer;

}

void Source::CancelUpdate(void)
{
	mb_cancel_update=1;
}
