// ProjectStatus.h
#pragma once

#include "QueryHit.h"
//#include "Query.h"

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

	void IncrementTrackQueryHitCount(int track);
	void IncrementFakeTrackQueryHitCount(int track);

	void ProjectName(char *project_name);
	char *ProjectName();

	// Public Data Members

	unsigned int m_total_query_hit_count;
	vector<unsigned int> v_fake_query_hit_counts;		// by track
	vector<unsigned int> v_query_hit_counts;	// by track

	vector<QueryHit> v_query_hits;
	unsigned int m_total_fake_query_hit_count;

private:
	char *p_project_name;
};