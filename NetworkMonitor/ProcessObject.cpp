#include "StdAfx.h"
#include "processobject.h"

ProcessObject::ProcessObject(const char* name)
{
	m_name=name;
}

ProcessObject::~ProcessObject(void)
{
}

bool ProcessObject::Update(void)
{
	for(int i=0;i<(int)mv_racks.Size();i++){
		RackProcessHolder* rph=(RackProcessHolder*)mv_racks.Get(i);
		if(rph->IsExpired()){
			mv_racks.Remove(i);
			i--;
		}
	}

	if(mv_racks.Size()>0)
		return true;  //this process still exists
	else return false;  //this process no longer exists
}

bool ProcessObject::AddRack(const char* ip)
{
	int low=0;
	int high=max(0,mv_racks.Size()-1);
	int index=0;
	//do a binary search to determine where we should insert this file into the array
	for(;low<high;){
		index=low+((high-low)>>1);
		RackProcessHolder* rack=(RackProcessHolder*)mv_racks.Get(index);
		int result=stricmp(ip,rack->m_ip.c_str());
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

	if(mv_racks.Size()>0){
		RackProcessHolder* rack=(RackProcessHolder*)mv_racks.Get(index);
		if(stricmp(ip,rack->m_ip.c_str())==0){
			rack->Renew();
			return false;
		}

		//this inefficiency is imposed on us by the premise that we can theoretically have duplicate hashes (files of different sizes actually having the same hash)
		for(int i=low;i<=high;i++){
			RackProcessHolder* rack=(RackProcessHolder*)mv_racks.Get(i);
			if(stricmp(ip,rack->m_ip.c_str())==0){
				rack->Renew();
				return false;
			}
		}
	}

	RackProcessHolder* rack=new RackProcessHolder(ip);
	mv_racks.Add(rack);
	mv_racks.Sort(1);
	return true;
}
