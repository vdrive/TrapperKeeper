#include "StdAfx.h"
#include "fileindexsystem.h"

FileIndexSystem::FileIndexSystem(void)
{
}

FileIndexSystem::~FileIndexSystem(void)
{
}

//this is our most inefficient function, and unfortunately it will be called often.
//though its not overly inefficent because the hosts are storing the files in a sorted array and doing binary searches to look them up.
UINT FileIndexSystem::GetHostsForFile(const char* hash, UINT size,vector<string> &v_ips)
{
	CSingleLock lock(&m_lock,TRUE);
	//build a list of hosts with a given file.
	UINT the_size=size;
	for(int i=0;i<(int)mv_hosts.Size();i++){
		FileHost* host=(FileHost*)mv_hosts.Get(i);
		if(host->FindFile(hash,the_size)){  //if it finds it, it'll also modify size to be the actual size of the file in case it wasn't
			v_ips.push_back(string(host->GetIP()));
		}
	}
	return the_size;
}

//called to tell a host that it is time to update all the files
void FileIndexSystem::UpdateHost(const char* ip, vector<string>& v_file_hashes, vector<int>& v_file_sizes)
{
	CSingleLock lock(&m_lock,TRUE);
	FileHost *host=GetHost(ip);
	if(host){
		host->UpdateFiles(v_file_hashes,v_file_sizes);
	}
	else{
		host=new FileHost(ip);
		host->UpdateFiles(v_file_hashes,v_file_sizes);
		mv_hosts.Add(host);
		while(mv_hosts.Size()>500){
			mv_hosts.Remove(0);
		}
		mv_hosts.Sort(1);
	}
}


void FileIndexSystem::Update(void)
{
	CSingleLock lock(&m_lock,TRUE);
	//check if any hosts have expired.
	for(int i=0;i<(int)mv_hosts.Size();i++){
		FileHost* host=(FileHost*)mv_hosts.Get(i);
		if(host->IsExpired()){
			mv_hosts.Remove(i);
			break;  //its ok to only delete one at a time.  plus is spreads work out over time.
		}
	}
}

FileHost* FileIndexSystem::GetHost(const char* ip)
{
	CSingleLock lock(&m_lock,TRUE);
	int low=0;
	int high=max(0,mv_hosts.Size()-1);
	int index=0;
	//do a binary search to determine where we should insert this file into the array
	for(;low<high;){
		index=low+((high-low)>>1);
		FileHost* fh=(FileHost*)mv_hosts.Get(index);
		int result=stricmp(ip,fh->GetIP());
		if(result<0){
			high=index-1;  //we know that our bounds are limited to the middle-1.
		}
		else if(result>0){
			low=index+1;
		}
		else{  
			break;
		}
	}

	if(mv_hosts.Size()>0){
		FileHost* fh=(FileHost*)mv_hosts.Get(index);
		if(fh->IsHost(ip)){
			return fh;
		}

		//this inefficiency is imposed on us by the premise that we can theoretically have duplicate hashes (files of different sizes actually having the same hash)
		for(int i=low;i<=high;i++){
			FileHost* fh=(FileHost*)mv_hosts.Get(i);
			if(fh->IsHost(ip)){
				return fh;
			}
		}
	}
	return NULL;
}
