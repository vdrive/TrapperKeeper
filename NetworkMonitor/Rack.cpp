#include "StdAfx.h"
#include "rack.h"
#include "DLLObject.h"
#include "ProcessObject.h"

Rack::Rack(const char* ip)
{
	m_ip=ip;
	mb_exist_check=true;
	m_next_thin_time=CTime::GetCurrentTime()+CTimeSpan(0,0,10+rand()%30,0);
	m_next_dll_request=CTime::GetCurrentTime()+CTimeSpan(0,0,0,rand()%60);
	m_next_process_request=CTime::GetCurrentTime()+CTimeSpan(0,0,0,rand()%60);
	mp_owner_network=NULL;
	mb_restart=false;
	m_version=0;
}

Rack::~Rack(void)
{
}

void Rack::Update(void)
{

	
	//response ping thinning algorithm
	if(CTime::GetCurrentTime()>m_next_thin_time){
		
		for(int i=2;i<(int)mv_ping_responses.Size();i++){
			PingResponse *pr=(PingResponse*)mv_ping_responses.Get(i);

			if(pr->m_response_time<CTime::GetCurrentTime()-CTimeSpan(20,0,0,0)){
				CSingleLock lock(&m_ping_vector_lock,TRUE);
				mv_ping_responses.Remove(i);
				i--;
				continue;  //discard a ping response if it is older than 30 days.  We don't need to remember this far back in time
			}

			PingResponse *mid_pr=(PingResponse*)mv_ping_responses.Get(i-1);
			PingResponse *last_pr=(PingResponse*)mv_ping_responses.Get(i-2);
			
			CTimeSpan dif_time=pr->m_response_time-last_pr->m_response_time;
			if(dif_time<CTimeSpan(0,0,40,0) && pr->mb_tk_on==mid_pr->mb_tk_on && pr->mb_tk_on==last_pr->mb_tk_on){
				CSingleLock lock(&m_ping_vector_lock,TRUE);
				mv_ping_responses.Remove(i-1);  //remove the middle one if it is unnecessary
				i--;
				continue;
			}
		}
		//schedule another thinning at some random time within the next 40 minutes.  Thus all racks don't need to run the thinning algorithm at the same time, this spreads work out
		m_next_thin_time=CTime::GetCurrentTime()+CTimeSpan(0,0,10+rand()%30,0);
	}
}

void Rack::EnumerateRackHistory(int range,vector <bool> &vb_on)
{
	CTime ref_time=CTime::GetCurrentTime();
	CTimeSpan one_hour(0,1,0,0);
	CSingleLock lock(&m_ping_vector_lock,TRUE);
	int response_offset=mv_ping_responses.Size()-1;
	//for each historical hour through range hours, determine if the trapper keeper was on or off
	for(int i=0;i<range;i++){
		bool b_on=false;
		for(int j=response_offset;j>=0;j--){
			PingResponse *pong=(PingResponse*)mv_ping_responses.Get(j);
			if(pong->m_response_time<=ref_time && pong->m_response_time>(ref_time-one_hour)){
				b_on=true;
				break;
			}
			else if(pong->m_response_time<(ref_time-one_hour)){
				break;  //aren't going to find one in the range we are looking for, since the response times are in chronological order
			}
			else if(pong->m_response_time>ref_time){
				response_offset=j;  //decrease our response_offset, so we don't waste work in the future looking at parts of the vector we know are too high, since ref_time is decreasing
			}
		}
		vb_on.push_back(b_on);
		ref_time-=one_hour;
	}
}

void Rack::EnumerateTKHistory(int range,vector <bool> &vb_on)
{
	CTime ref_time=CTime::GetCurrentTime();
	CTimeSpan one_hour(0,1,0,0);
	CSingleLock lock(&m_ping_vector_lock,TRUE);
	int response_offset=mv_ping_responses.Size()-1;
	//for each historical hour through range hours, determine if the trapper keeper was on or off
	for(int i=0;i<range;i++){
		bool b_on=false;
		for(int j=response_offset;j>=0;j--){
			PingResponse *pong=(PingResponse*)mv_ping_responses.Get(j);
			if(pong->m_response_time<=ref_time && pong->m_response_time>(ref_time-one_hour)){
				if(pong->mb_tk_on){
					b_on=true;
					break;
				}
			}
			else if(pong->m_response_time<(ref_time-one_hour)){
				break;  //aren't going to find one in the range we are looking for, since the response times are in chronological order
			}
			else if(pong->m_response_time>ref_time){
				response_offset=j;  //decrease our response_offset, so we don't waste work in the future looking at parts of the vector we know are too high, since ref_time is decreasing
			}
		}
		vb_on.push_back(b_on);
		ref_time-=one_hour;
	}
}

void Rack::GotPingResponse(bool b_trapper_on)
{
	mv_ping_responses.Add(new PingResponse(b_trapper_on));
}

bool Rack::IsAlive(void)
{
	CTime ref_time=CTime::GetCurrentTime();
	CTimeSpan five_minutes(0,0,5,0);
	CSingleLock lock(&m_ping_vector_lock,TRUE);
	for(int j=mv_ping_responses.Size()-1;j>=0;j--){
		PingResponse *pong=(PingResponse*)mv_ping_responses.Get(j);
		if(pong->m_response_time<=ref_time && pong->m_response_time>(ref_time-five_minutes)){
			return true;
		}
		else if(pong->m_response_time<(ref_time-five_minutes)){
			break;  //aren't going to find one in the range we are looking for, since the response times are in chronological order
		}
	}
	return false;
}

bool Rack::IsTKOn(void)
{
	CTime ref_time=CTime::GetCurrentTime();
	CTimeSpan five_minutes(0,0,5,0);

	CSingleLock lock(&m_ping_vector_lock,TRUE);
	for(int j=mv_ping_responses.Size()-1;j>=0;j--){
		PingResponse *pong=(PingResponse*)mv_ping_responses.Get(j);
		if(pong->m_response_time<=ref_time && pong->m_response_time>(ref_time-five_minutes)){
			if(pong->mb_tk_on)
				return true;
		}
		else if(pong->m_response_time<(ref_time-five_minutes)){
			break;  //aren't going to find one in the range we are looking for, since the response times are in chronological order
		}
	}
	return false;
}

bool Rack::SetDLLs(Vector& v_dlls)
{
	CSingleLock lock(&m_dll_vector_lock,TRUE);
	bool b_changed=false;
	if(v_dlls.Size()!=mv_dlls.Size())
		b_changed=true;

	if(!b_changed){
		for(int i=0;i<(int)v_dlls.Size();i++){
			if(v_dlls.Get(i)!=mv_dlls.Get(i))
				b_changed=true;
		}
	}
	mv_dlls.Clear();
	mv_dlls.Copy(&v_dlls);

	return b_changed;
}

bool Rack::HasDll(const char* name)
{
	for(int i=0;i<(int)mv_dlls.Size();i++){
		DLLObject *dll=(DLLObject*)mv_dlls.Get(i);
		if(stricmp(dll->m_name.c_str(),name)==0)
			return true;
	}
	return false;
}

void Rack::GetDLLs(Vector& v_dlls)
{
	v_dlls.Copy(&mv_dlls);
}

void Rack::GetPingResponseVector(Vector& v_pings)
{
	v_pings.Copy(&mv_ping_responses);
}

void Rack::SavePingResponse(PingResponse* pr)
{
	CSingleLock(&m_ping_vector_lock,TRUE);
	mv_ping_responses.Add(pr);
}
