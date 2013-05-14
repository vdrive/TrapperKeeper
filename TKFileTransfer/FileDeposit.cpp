#include "StdAfx.h"
#include "..\tkcom\SHA1.h"
#include "..\tkcom\Timer.h"
#include "filedeposit.h"
#include "FileBufferFile.h"

FileDeposit::FileDeposit()
{
	mb_need_file_update=false;
	mb_updated_once=false;
}

FileDeposit::~FileDeposit(void)
{
}

void FileDeposit::Update(const char* directory,vector <string> &v_file_names, vector <string> &v_full_file_paths,vector <string> &v_sha1,vector <int> &v_file_sizes){
	WIN32_FIND_DATA info;

	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	CString name;//=path+info.cFileName;
	//AddElement(map,(char*)(LPCSTR)name); //add the first element
	int file_count=0;
	TRACE("TKFileTransfer Service FileDeposit::Update() adding files in directory: %s.\n",directory);
	while(FindNextFile(hFind,&info) && !this->b_killThread){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		name=path+info.cFileName;

		if(((GetFileAttributes(name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){  //is it a directory?
			//its a directory
			Update(name,v_file_names,v_full_file_paths,v_sha1,v_file_sizes); //recursive call
		}
		else{
			file_count++;
			string sha1;
			bool b_hashed=CalculateFileSHA1(name,sha1);
			if(!b_hashed) 
				continue;  //failed to calculate the hash of the file for whatever reason, we'll pretend this file doesn't exist
			string file_name=info.cFileName;
			int size=info.nFileSizeLow;
			string full_path=(LPCSTR)name;

			v_file_names.push_back(file_name);
			v_full_file_paths.push_back(full_path);
			v_sha1.push_back(sha1);
			v_file_sizes.push_back(size);
		}
	}
	TRACE("TKFileTransfer Service FileDeposit::Update() Finished adding %d files in directory: %s.\n",file_count,directory);
	FindClose(hFind);	
}

UINT FileDeposit::Run(){
	Timer last_update;
	bool b_updated_once=false;  //a flag so we update right away upon start and don't wait forever to start a file scan
	TRACE("TKFileTransfer Service FileDeposit::Run().\n");
	while(!this->b_killThread){  //run forever
		if((!b_updated_once && last_update.HasTimedOut(10)) || last_update.HasTimedOut(60*20)){  //every 20 minutes we begin anew
			
			TRACE("TKFileTransfer Service FileDeposit::Run() Checking File Deposit for new files.\n");
			b_updated_once=true;
			vector <string> v_file_names;
			vector <string> v_full_file_paths;
			vector <string> v_sha1;
			vector <int> v_file_sizes;
			

			Update("c:\\syncher\\file deposit",v_file_names,v_full_file_paths,v_sha1,v_file_sizes);
			Update("c:\\syncher\\src",v_file_names,v_full_file_paths,v_sha1,v_file_sizes);

			CSingleLock lock(&m_data_lock,TRUE);
			bool b_changed=false;

			//do a quick check on the size of the vector
			if(mv_file_names.size()!=v_file_names.size()){
				b_changed=true;
			}

			if(!b_changed && !this->b_killThread){  //do an exhaustive check to see if any file in the system has changed
				for(int i=0;i<(int)v_file_names.size();i++){
					if(stricmp(v_file_names[i].c_str(),mv_file_names[i].c_str())!=0){  //is there any difference in file name?
						b_changed=true;
						break;
					}
					else if(stricmp(v_sha1[i].c_str(),mv_sha1[i].c_str())!=0){  //is there any difference in sha1?
						b_changed=true;
						break;
					}
					else if(v_file_sizes[i]!=mv_file_sizes[i]){  //is there any difference in sha1?
						b_changed=true;
						break;
					}
				}
			}
			
			if(b_changed){
				mv_file_names=v_file_names;
				mv_sha1=v_sha1;
				mv_full_file_paths=v_full_file_paths;
				mv_file_sizes=v_file_sizes;
			}
			
			last_update.Refresh();

			if(!this->b_killThread && (b_changed)){  //if we have changed we need to report those changes to the database
				mb_need_file_update=true;
				TRACE("TKFileTransfer Service FileDeposit::Run() files have changed, flagging.\n");
			}
			mb_updated_once=true;
		}
		Sleep(100);
	}
	return 0;
}

bool FileDeposit::CalculateFileSHA1(const char* file_name,string &hash)
{
	char ba[33];
	ba[0]='\0';
	SHA1 sha1;
	if(mb_updated_once){  //we have already done an initial scan, this is just after the fact
		if(sha1.HashFile((char*)file_name,100,this->b_killThread)){
			sha1.Final();
			sha1.Base32Encode(ba);
		}
		else
			return false;
	}
	else{
		if(sha1.HashFile((char*)file_name,0,this->b_killThread)){
			sha1.Final();
			sha1.Base32Encode(ba);
		}
		else return false;
	}
	hash=string(ba);
	return true;
}

void FileDeposit::EnumerateFiles(vector<string> & v_file_hashes, vector<int> & v_file_sizes)
{
	CSingleLock lock(&m_data_lock,TRUE);
	for(int i=0;i<(int)mv_file_names.size();i++){
		v_file_hashes.push_back(mv_sha1[i]);
		v_file_sizes.push_back(mv_file_sizes[i]);
	}
}

int FileDeposit::GetFilePart(const char* hash, UINT file_size, byte* buffer,UINT part)
{
	UINT start=part*PARTSIZE;
	CSingleLock lock(&m_data_lock,TRUE);
	int len=(int)mv_sha1.size();
	for(int i=0;i<len;i++){
		if(stricmp(mv_sha1[i].c_str(),hash)==0 && mv_file_sizes[i]==file_size){
			//The file deposit does have this hash, lets extract it
			HANDLE file = CreateFile(mv_full_file_paths[i].c_str(), // open file at local_path 
						GENERIC_READ,              // open for reading 
						FILE_SHARE_READ,           // share for reading 
						NULL,                      // no security 
						OPEN_EXISTING,             // existing file only 
						FILE_ATTRIBUTE_NORMAL,     // normal file 
						NULL);                     // no attr. template 

			if(file==INVALID_HANDLE_VALUE || file==NULL){
				TRACE("TKFileTransfer FileDeposit:  Hash found, but unable to open file %s.\n",mv_full_file_paths[i].c_str());
				return -1;
			}

			LONG high_move=0;  //if we pass in 0 instead of an actual long address, then low_move gets treated as a signed value and we can't work with sizes between 2 gigs and 4 gigs.
			LONG low_move=start;  
			DWORD dwPtr=SetFilePointer(file,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading
			DWORD error;
			if(dwPtr == INVALID_SET_FILE_POINTER){  //test for a failure
				error=GetLastError();
				CloseHandle(file);
				ASSERT(0);
				return -1;
			}

			DWORD num_read=0;  //a variable that will hold how much data was read off the disk
			BOOL stat=ReadFile(file,buffer,PARTSIZE,&num_read,NULL);
			
			DWORD fs=0,fsh=0;
			fs=GetFileSize(file,&fsh);
			CloseHandle(file);

			//do error checking
			if(start>fs)
				return -1;
			else if(num_read<PARTSIZE && (fs-start)!=num_read)
				return -1;
			else
				return num_read;
		}
	}
	return -1;
}

bool FileDeposit::GetFileStatusAsString(const char* sha1, UINT file_size, string &completed)
{
	CSingleLock lock(&m_data_lock,TRUE);
	for(int i=0;i<(int)mv_sha1.size();i++){
		if(stricmp(mv_sha1[i].c_str(),sha1)==0 && mv_file_sizes[i]==file_size){
			int num_parts=file_size/PARTSIZE;
			if(num_parts==0 || (file_size%PARTSIZE)!=0){
				num_parts++;
			}
			int len=num_parts/8;
			if((num_parts%8)!=0){
				len++;
			}
			for(int i=0;i<(int)len;i++){
				byte the_val=0xff;
				byte the_val1=(the_val>>4)&0xf;
				byte the_val2=(the_val)&0xf;
				char ch1=Utility::ByteToChar(the_val1);
				char ch2=Utility::ByteToChar(the_val2);
				completed+=ch1;
				completed+=ch2;
			}
			return true;
		}
	}
	return false;
}
