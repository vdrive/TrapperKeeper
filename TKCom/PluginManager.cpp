#include "StdAfx.h"
#include "pluginmanager.h"
#include "Timer.h"

#define MAXQUEUESIZE (1<<23)  //a plugin may only have up to 8 megs backed up.

PluginManager::PluginManager(void)
{
	m_queued_data=0;
}

PluginManager::~PluginManager(void)
{
}

bool PluginManager::QueueSend(DataBuffer* buffer, vector <string> &v_dests,UINT from_app)
{
	CSingleLock lock(&m_job_lock,TRUE);

	if(m_queued_data>50000000) //50 meg queue is max
		return false;


	m_queued_data+=buffer->GetDataLength();
	
	//UINT limit=MAXQUEUESIZE;
	//if(from_app==62)
	//	limit*=5; //file transfer gets to cheat
	//if(from_app==63)
	//	limit*=5; //syncher gets to cheat

	//if(CalculateAppQueueSize(from_app)>(limit)){ //is app trying to send too much at once?
	//	delete buffer;  //we are responsible for managing the DataBuffers passed in.
	//	return false;
	//}

	PluginManager::SendJob *job=new PluginManager::SendJob(buffer,from_app);
	for(UINT i=0;i<v_dests.size();i++){  //destinations this 1 Databuffer is to be sent to.
		job->AddDest(v_dests[i].c_str());
	}
	ml_jobs.Add(job);  //add yet another job to be dispatched;
	return true;
}

//call to calculate the job with highest priority.  (it has been waiting the longest)
//int PluginManager::GetHighestPriorityJob(void)
//{
/*	int max_priority=-1;  //any job will have a priority higher than this eh.
	int max_index=-1;  //index of the job with the max priority.  it will get reassign as the loop runs.

	//go through each of the jobs and get its priority.  compare it to the one that currently has the highest priority and reassign if necessary.
	UINT len=mv_jobs.Size();
	for(UINT i=0;i<len;i++){
		PluginManager::SendJob *job=(PluginManager::SendJob*)mv_jobs.Get(i);
		int jp=(int)job->GetPriority();
		if(jp>max_priority){
			max_index=(int)i;
			max_priority=jp;
		}
	}	

	//TRACE("Max priority was: %d\n",max_priority);
	return max_index;*/
	//return ml_jobs.Size()-1;
//}

void PluginManager::Process(ComTCP* tcp)
{
	//static Timer memory_check;
	//if(memory_check.HasTimedOut(60)){
	//	TRACE("Com:  PluginManager:  mv_jobs.Size()=%d\n",mv_jobs.Size());
	//	memory_check.Refresh();
	//}
	//int len=mv_jobs.Size();
	//int count=0;
	//static UINT scan_pos=0;
	//if(scan_pos>=mv_jobs.Size())
	//	scan_pos=0;

	CSingleLock lock(&m_job_lock,TRUE);
	ml_jobs.StartIteration();
	while(true){
		PluginManager::SendJob *job=(PluginManager::SendJob*)ml_jobs.GetCurrent();
		if(!job){
			return;
		}

		job->ml_destinations.StartIteration();
		while(true){
			Destination* dest=(Destination*)job->ml_destinations.GetCurrent();
			if(!dest)
				break;

			if(dest->m_last_attempt<(GetTickCount()-400)){
				if(tcp->CanSend(dest->m_dest.c_str(),job->GetData())){
					job->ml_destinations.RemoveCurrentAndAdvance();
					continue;
				}
				else{
					dest->m_attempts++;
					if(dest->m_attempts>30){
						job->ml_destinations.RemoveCurrentAndAdvance();
					}
					else{
						dest->m_last_attempt=GetTickCount();
						job->ml_destinations.Advance();
					}
				}
			}
			else //to early to try to resend this job
				job->ml_destinations.Advance();
			
		}

		if(job->ml_destinations.Size()==0){ //does this job have any more destinations to send too?
			ml_jobs.RemoveCurrentAndAdvance();
			continue;
		}
		else ml_jobs.Advance();

		/*
		job->Renew();  //lower this jobs priority
		DataBuffer *data=job->GetData();
		const char *dest=job->GetDest();

		if(!dest){  //no destinations left.
			m_queued_data-=data->GetDataLength();
			CSingleLock lock(&m_job_lock,TRUE);
			mv_jobs.Remove(scan_pos);
			//if(scan_pos-1<mv_jobs.Size())
			//	scan_pos++;
		}
		else if(tcp->CanSend(dest,data)){  //if we can send this high priority job now...
			job->PopDest(); //pop that destination off, because we can send it.
		}
		else{ //else we aren't going to try to send anything else until we can send the first high priority job that needs to be sent.
			job->RotateDest();  //try sending to another of this jobs destinations
		}		*/
	}


	/*
	for(UINT scan_pos=mv_jobs.Size()-1;scan_pos>=0;scan_pos--){  //give each job a chance to dispatch
	//for(UINT scan_pos=0;scan_pos<mv_jobs.Size();scan_pos++){  //give each job a chance to dispatch
		//get the highest priority job
		
		//int index=GetHighestPriorityJob();
		//if(index<0) //no pending messages
		//	return; 

		PluginManager::SendJob *job=(PluginManager::SendJob*)mv_jobs.Get(scan_pos);
		if(!job){
			return;
		}

		job->Renew();  //lower this jobs priority
		DataBuffer *data=job->GetData();
		const char *dest=job->GetDest();

		if(!dest){  //no destinations left.
			m_queued_data-=data->GetDataLength();
			CSingleLock lock(&m_job_lock,TRUE);
			mv_jobs.Remove(scan_pos);
			//if(scan_pos-1<mv_jobs.Size())
			//	scan_pos++;
		}
		else if(tcp->CanSend(dest,data)){  //if we can send this high priority job now...
			job->PopDest(); //pop that destination off, because we can send it.
		}
		else{ //else we aren't going to try to send anything else until we can send the first high priority job that needs to be sent.
			job->RotateDest();  //try sending to another of this jobs destinations
		}
	}
	*/
}

PluginManager::SendJob::SendJob( DataBuffer *_data,UINT _from_app){
	data=_data;
	data->ref();  //tell it we are referencing it.
	from_app=_from_app;
}

PluginManager::SendJob::~SendJob(){
	data->deref();  //tell it we are no longer referencing it.  it will free itself when any vectors etc are done referencing it.
}


void PluginManager::SendJob::AddDest(const char* dest){
	ml_destinations.Add(new Destination(dest));
}

bool PluginManager::SendJob::IsFromApp(UINT app){
	return (from_app==app)?true:false;
}

UINT PluginManager::SendJob::GetDataSize(){
	return data->GetDataLength();
}

DataBuffer* PluginManager::SendJob::GetData(){
	return data;
}



bool PluginManager::SendJob::IsDone(){
	return (ml_destinations.Size()>0)?true:false;
}