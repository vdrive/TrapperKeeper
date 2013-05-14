// ProjectStatus.cpp

#include "stdafx.h"
#include "ProjectStatus.h"

//
//
//
ProjectStatus::ProjectStatus()
{
	p_project_name=NULL;

	Clear();
}

//
// Copy constructor
//
ProjectStatus::ProjectStatus(const ProjectStatus &status)
{
	p_project_name=NULL;

	p_project_name=new char[strlen(status.p_project_name)+1];
	strcpy(p_project_name,status.p_project_name);

	m_total_query_count=status.m_total_query_count;
//	m_total_query_hit_count=status.m_total_query_hit_count;
	
	v_query_counts=status.v_query_counts;
//	v_query_hit_counts=status.v_query_hit_counts;

//	v_query_hits=status.v_query_hits;
	v_queries=status.v_queries;

	m_spoof_count=status.m_spoof_count;
	m_dist_count=status.m_dist_count;
	m_poison_count=status.m_poison_count;
}

//
//
//
ProjectStatus::~ProjectStatus()
{
	if(p_project_name!=NULL)
	{
		delete [] p_project_name;
		p_project_name=NULL;
	}
}

//
//
//
void ProjectStatus::Clear()
{
	if(p_project_name!=NULL)
	{
		delete [] p_project_name;
		p_project_name=NULL;
	}

	ClearCounters();
}

//
// Assignment operator
//
ProjectStatus& ProjectStatus::operator=(const ProjectStatus &status)
{
	if(p_project_name!=NULL)
	{
		delete [] p_project_name;
		p_project_name=NULL;
	}
	
	p_project_name=new char[strlen(status.p_project_name)+1];
	strcpy(p_project_name,status.p_project_name);

	m_total_query_count=status.m_total_query_count;
//	m_total_query_hit_count=status.m_total_query_hit_count;
	
	v_query_counts=status.v_query_counts;
//	v_query_hit_counts=status.v_query_hit_counts;

//	v_query_hits=status.v_query_hits;
	v_queries=status.v_queries;

	m_spoof_count=status.m_spoof_count;
	m_dist_count=status.m_dist_count;
	m_poison_count=status.m_poison_count;

	return *this;
}

//
//
//
void ProjectStatus::ClearCounters()
{
	m_total_query_count=0;
//	m_total_query_hit_count=0;

	v_query_counts.clear();
//	v_query_hit_counts.clear();

//	v_query_hits.clear();
	v_queries.clear();

	m_spoof_count=0;
	m_dist_count=0;
	m_poison_count=0;
}

//
//
//
void ProjectStatus::IncrementTrackQueryCount(int track)
{
	m_total_query_count++;

	// See if we need to create entries to store this track's count
	while(v_query_counts.size()<(UINT)track+1)
	{
		v_query_counts.push_back(0);
	}
	
	// Increment the counter for this track
	v_query_counts[track]++;
}

//
//
//
/*
void ProjectStatus::IncrementTrackQueryHitCount(int track)
{
	m_total_query_hit_count++;

	// See if we need to create entries to store this track's count
	while(v_query_hit_counts.size()<(UINT)track+1)
	{
		v_query_hit_counts.push_back(0);
	}
	
	// Increment the counter for this track
	v_query_hit_counts[track]++;
}
*/

//
//
//
void ProjectStatus::ProjectName(char *project_name)
{
	if(p_project_name!=NULL)
	{
		delete [] p_project_name;
		p_project_name=NULL;
	}

	p_project_name=new char[strlen(project_name)+1];
	strcpy(p_project_name,project_name);
}

//
//
//
char *ProjectStatus::ProjectName()
{
	return p_project_name;
}