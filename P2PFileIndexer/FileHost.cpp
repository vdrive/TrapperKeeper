#include "StdAfx.h"
#include "filehost.h"

FileHost::FileHost(const char *ip)
{
	m_ip=ip;
}

FileHost::~FileHost(void)
{
}

bool FileHost::IsHost(const char* ip)
{
	if(stricmp(ip,m_ip.c_str())==0)
		return true;
	else
		return false;
}

void FileHost::RemoveFile(FileObject* file)
{
}

const char* FileHost::GetIP(void)
{
	return m_ip.c_str();
}

bool FileHost::FindFile(const char* hash,UINT &size)
{
	int low=0;
	int high=max(0,mv_files.Size()-1);
	int index=0;
	//do a binary search to determine where we should insert this file into the array
	for(;low<high;){
		index=low+((high-low)>>1);
		FileObject* fo=(FileObject*)mv_files.Get(index);
		int result=stricmp(hash,fo->GetHash());
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

	if(mv_files.Size()>0){
		FileObject* fo=(FileObject*)mv_files.Get(index);
		if(fo->IsFile(hash,size)){
			size=fo->GetSize(); //record the size in case the requesting party doesn't yet know the size
			return true;
		}

		//this inefficiency is imposed on us by the premise that we can theoretically have duplicate hashes (files of different sizes actually having the same hash)
		for(int i=low;i<=high;i++){
			fo=(FileObject*)mv_files.Get(i);
			if(fo->IsFile(hash,size)){
				size=fo->GetSize();  //record the size in case the requesting party doesn't yet know the size
				return true;
			}
		}
	}
	return false;
}

void FileHost::UpdateFiles(vector<string>& v_file_hashes, vector<int>& v_file_sizes)
{
	mv_files.Clear();

	for(int i=0;i<(int)v_file_hashes.size();i++){
		const char* hash=v_file_hashes[i].c_str();
		UINT file_size=v_file_sizes[i];
		FileObject *fo=new FileObject(hash,file_size);
		mv_files.Add(fo);
	}
	mv_files.Sort(true);
	m_last_update.Refresh();
}

bool FileHost::IsExpired(void)
{
	if(m_last_update.HasTimedOut(60*60*12)){  //if we haven't heard from this host in 12 hours, lets kill him
		return true;
	}
	else{
		return false;
	}
}
