#include "StdAfx.h"
#include "p2pcom.h"
#include "..\tkfiletransfer\tkfiletransferinterface.h"
#include <mmsystem.h>

P2PCom::P2PCom(void)
{
}

P2PCom::~P2PCom(void)
{
}

void P2PCom::DataReceived(char *source_ip, void *data, UINT data_length){	
	TKFileTransferInterface::IPInterface message;

	if(data_length<2)
		return;

	//TRACE("P2PIndexingService P2PCom::DataReceived() About to examine buffer from source=%s.\n",source_ip);
	message.ReadFromBuffer((byte*)data);
	if(message.m_type==message.P2PFILELIST){
		TRACE("P2PIndexingService P2PCom::DataReceived() Received p2p file list from source=%s.\n",source_ip);
		TKFileTransferInterface::IPInterfaceDataSet file_list;
		UINT tmp;
 		file_list.ReadFromBuffer(message.GetData(tmp));
		m_file_index_system.UpdateHost(source_ip,file_list.v_strings,file_list.v_ints);
	}
	else if(message.m_type==message.P2PGETFILEHOSTS){  //handling a ton of these will be the most taxing part of the whole system
		if(mv_send_jobs.Size()<500){
			P2PCom::SendJob *job=new P2PCom::SendJob();
			job->m_dest=source_ip;
			job->m_hash=message.v_strings[0];
			job->m_size=message.v_ints[0];
			mv_send_jobs.Add(job);
		}
	}
}

UINT P2PCom::Run(){
	Timer m_last_cache_update;
	Timer m_last_index_system_update;
	while(!this->b_killThread){
		if(m_last_cache_update.HasTimedOut(120)){
			for(int i=0;i<(int)mv_cached_sources.Size();i++){
				CachedSource *src=(CachedSource*)mv_cached_sources.Get(i);
				if(src->m_age.HasTimedOut(300)){  //we only cache a hashes sources for 5 minutes and then we discard it.
					mv_cached_sources.Remove(i);
					i--;
				}
			}
			m_last_cache_update.Refresh();
		}
		if(m_last_index_system_update.HasTimedOut(120)){
			m_file_index_system.Update();
			m_last_index_system_update.Refresh();
		}
		if(mv_send_jobs.Size()>0){
			P2PCom::SendJob *job=(P2PCom::SendJob *)mv_send_jobs.Get(0);
			TKFileTransferInterface::IPInterface response;
			response.m_type=response.P2PFILEHOSTS;
			response.v_strings.push_back(job->m_hash);  //push the requested hash on the top
			//TRACE("P2PIndexingService P2PCom::DataReceived() %s asking for hosts for file %s.\n",source_ip,message.v_strings[0].c_str());
			//response.v_ints.push_back(message.v_ints[0]);  //push the requested size on the top
			
			vector<string> v_ips;

			bool b_is_cached=false;
			CachedSource *cs=GetCachedSource(job->m_hash.c_str(),job->m_size);
			if(cs){
				response.v_ints.push_back(cs->m_file_size);  //push the known size onto the response, in case the requestor doesn't yet know the size
				b_is_cached=true;
				//TRACE("P2PIndexingService P2PCom::DataReceived() %s asked for hosts for file %s, using %d cached hosts.\n",source_ip,message.v_strings[0].c_str(),src->mv_ips.size());
				v_ips=cs->mv_ips;  //
			}

			int ip_count=(int)v_ips.size();

			//push all the host ips on next
			if(!b_is_cached){
				vector <string> v_tmp;
				UINT file_size=m_file_index_system.GetHostsForFile(job->m_hash.c_str(),job->m_size,v_tmp);
				response.v_ints.push_back(file_size);  //push the known size onto the response, in case the requestor doesn't yet know the size
				srand(timeGetTime());

				int len=(int)v_tmp.size();
				//choose up to 40 random sources to give out for now
				ip_count=0;
				for(int i=0;i<40 && i<len;i++,ip_count++){
					int index=rand()%(int)v_tmp.size();
					v_ips.push_back(v_tmp[index]);  //push another random, but unique ip onto our ip vector
					v_tmp.erase(v_tmp.begin()+index);  
				}
				//TRACE("P2PIndexingService P2PCom::DataReceived() %s asked for hosts for file %s, found %d new hosts.  temporarily caching these results\n",source_ip,message.v_strings[0].c_str(),v_ips.size());

				CachedSource* ncs=new CachedSource(job->m_hash.c_str(),file_size);
				mv_cached_sources.Add(ncs);
				mv_cached_sources.Sort(1);
				ncs->mv_ips=v_ips;  //store those 40- ips into our cache
			}

			for(int i=0;i<(int)v_ips.size();i++){
				response.v_strings.push_back(v_ips[i]);  //store those ips onto our response
			}

			if(ip_count>0){  //if there are currently any hosts for this file...
				//send a response listing the hosts for this particular hash
				int init_buf_size=20*((int)response.v_strings.size());
				byte *buf=new byte[64000];
				int len=response.WriteToBuffer(buf);
				
				this->SendReliableData((char*)job->m_dest.c_str(),buf,len);
				delete []buf;
			}			
			mv_send_jobs.Remove(0);
		}
		Sleep(10);
	}
	return 0;
}

void P2PCom::Update(void)
{
	//TRACE("P2PIndexingService P2PCom::Update().\n");
	
}

P2PCom::CachedSource* P2PCom::GetCachedSource(const char* hash,UINT size)
{
	int low=0;
	int high=max(0,mv_cached_sources.Size()-1);
	int index=0;
	//do a binary search to determine where we should insert this file into the array
	for(;low<high;){
		index=low+((high-low)>>1);
		CachedSource* cs=(CachedSource*)mv_cached_sources.Get(index);
		int result=stricmp(hash,cs->m_hash.c_str());
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

	if(mv_cached_sources.Size()>0){
		CachedSource* cs=(CachedSource*)mv_cached_sources.Get(index);
		if(stricmp(hash,cs->m_hash.c_str())==0 && (size==cs->m_file_size || size==-1)){
			return cs;
		}

		//this inefficiency is imposed on us by the premise that we can theoretically have duplicate hashes (files of different sizes actually having the same hash)
		for(int i=low;i<=high;i++){
			CachedSource* cs=(CachedSource*)mv_cached_sources.Get(i);
			if(stricmp(hash,cs->m_hash.c_str())==0 && (size==cs->m_file_size || size==-1)){
				return cs;
			}
		}
	}
	return NULL;
}
