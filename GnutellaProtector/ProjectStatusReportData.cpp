// ProjectStatusReportData.cpp

#include "stdafx.h"
#include "ProjectStatusReportData.h"

//
//
//
ProjectStatusReportData::ProjectStatusReportData()
{
	Clear();
}

//
//
//
ProjectStatusReportData::~ProjectStatusReportData()
{
}

//
//
//
void ProjectStatusReportData::Clear()
{
	m_project_name.erase();
	v_track_query_count.clear();
	v_track_query_hit_count.clear();
	m_spoof_count=0;
	m_dist_count=0;
	m_poison_count=0;
	memset(&m_start_time,0,sizeof(CTime));
	memset(&m_end_time,0,sizeof(CTime));
}

//
//
//
int ProjectStatusReportData::GetBufferLength()
{
	int len=0;

	len+=(int)m_project_name.size()+1;
	len+=sizeof(unsigned int);	// size of track query count vector
	len+=sizeof(unsigned int)*(int)v_track_query_count.size();
	len+=sizeof(unsigned int);	// size of track query count vector
	len+=sizeof(unsigned int)*(int)v_track_query_hit_count.size();
	len+=sizeof(unsigned int);	// spoof count
	len+=sizeof(unsigned int);	// dist count
	len+=sizeof(unsigned int);	// poison count
	len+=sizeof(CTime);	// start time
	len+=sizeof(CTime);	// end time

	return len;
}

//
//
//
int ProjectStatusReportData::WriteToBuffer(char *buf)
{
	UINT i;

	char *ptr=buf;

	memset(ptr,0,GetBufferLength());

	strcpy(ptr,m_project_name.c_str());
	ptr+=m_project_name.size()+1;

	*((unsigned int *)ptr)=(UINT)v_track_query_count.size();
	ptr+=sizeof(unsigned int);
	for(i=0;i<v_track_query_count.size();i++)
	{
		*((unsigned int *)ptr)=v_track_query_count[i];
		ptr+=sizeof(unsigned int);
	}

	*((unsigned int *)ptr)=(UINT)v_track_query_hit_count.size();
	ptr+=sizeof(unsigned int);
	for(i=0;i<v_track_query_hit_count.size();i++)
	{
		*((unsigned int *)ptr)=v_track_query_hit_count[i];
		ptr+=sizeof(unsigned int);
	}

	*((unsigned int *)ptr)=m_spoof_count;
	ptr+=sizeof(unsigned int);

	*((unsigned int *)ptr)=m_dist_count;
	ptr+=sizeof(unsigned int);

	*((unsigned int *)ptr)=m_poison_count;
	ptr+=sizeof(unsigned int);

	*((CTime *)ptr)=m_start_time;
	ptr+=sizeof(CTime);

	*((CTime *)ptr)=m_end_time;
	ptr+=sizeof(CTime);

	return GetBufferLength();
}

//
//
//
int ProjectStatusReportData::ReadFromBuffer(char *buf)
{
	UINT i;
	unsigned int num;
	
	char *ptr=buf;

	m_project_name=ptr;
	ptr+=strlen(ptr)+1;

	num=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	for(i=0;i<num;i++)
	{
		v_track_query_count.push_back(*((unsigned int *)ptr));
		ptr+=sizeof(unsigned int);
	}

	num=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	for(i=0;i<num;i++)
	{
		v_track_query_hit_count.push_back(*((unsigned int *)ptr));
		ptr+=sizeof(unsigned int);
	}

	m_spoof_count=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	
	m_dist_count=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_poison_count=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_start_time=*((CTime *)ptr);
	ptr+=sizeof(CTime);

	m_end_time=*((CTime *)ptr);
	ptr+=sizeof(CTime);

	return GetBufferLength();
}

//
//
//
void ProjectStatusReportData::IncrementTrackCounters(ProjectStatus &status)
{
	UINT i;

	// If the size of the status vector is > than this one, make this one bigger, and then add all the values up

	// Track Queries
	while(status.v_query_counts.size()>v_track_query_count.size())
	{
		v_track_query_count.push_back(0);
	}

	for(i=0;i<status.v_query_counts.size();i++)
	{
		v_track_query_count[i]+=status.v_query_counts[i];
	}
/*
	// Track Query Hits
	while(status.v_query_hit_counts.size()>v_track_query_hit_count.size())
	{
		v_track_query_hit_count.push_back(0);
	}

	for(i=0;i<status.v_query_hit_counts.size();i++)
	{
		v_track_query_hit_count[i]+=status.v_query_hit_counts[i];
	}
*/
}