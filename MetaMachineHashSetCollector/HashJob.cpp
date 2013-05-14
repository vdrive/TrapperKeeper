#include "StdAfx.h"
#include "hashjob.h"

HashJob::HashJob(void)
{
}

HashJob::~HashJob(void)
{
}

//parse out duplicate ips.
void HashJob::ClearDuplicates(void)
{
	for(int i=0;i<(int)mv_ips.size();i++){
		for(int j=i+1;j<(int)mv_ips.size();j++){
			if(stricmp(mv_ips[i].c_str(),mv_ips[j].c_str())==0){
				mv_ips.erase(mv_ips.begin()+j);
				j--;
			}
		}
	}
}
