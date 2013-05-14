// ProjectStatus.h
#pragma once

//#include "QueryHit.h"
#include "Query.h"

class ProjectStatus
{
public:
	// Public Member Functions
	ProjectStatus();
	ProjectStatus(const ProjectStatus &status);	// copy constructor
	~ProjectStatus();
	void Clear();
	void ClearCounters();

	ProjectStatus& operator=(const ProjectStatus &status);	// assignment operator

	void IncrementTrackQueryCount(int track);
//	void IncrementTrackQueryHitCount(int track);

	void ProjectName(char *project_name);
	char *ProjectName();

	// Public Data Members
//	string m_project_name;

	unsigned int m_total_query_count;
//	unsigned int m_total_query_hit_count;
	vector<unsigned int> v_query_counts;		// by track
//	vector<unsigned int> v_query_hit_counts;	// by track

//	vector<QueryHit> v_query_hits;
	vector<Query> v_queries;

	unsigned int m_spoof_count;
	unsigned int m_dist_count;
	unsigned int m_poison_count;

private:
	char *p_project_name;
};