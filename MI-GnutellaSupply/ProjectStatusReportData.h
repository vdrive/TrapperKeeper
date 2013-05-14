// ProjectStatusReportData.h
#pragma once

#include "ProjectDataStructure.h"
#include "ProjectStatus.h"

class ProjectStatusReportData : public ProjectDataStructure
{
public:
	// Public Member Functions
	ProjectStatusReportData();
	~ProjectStatusReportData();
	void Clear();

	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);	// returns buffer length

	void IncrementTrackCounters(ProjectStatus &status);

	// Public Data Members
	string m_project_name;
	vector<unsigned int> v_track_query_count;
	vector<unsigned int> v_track_query_hit_count;
	unsigned int m_spoof_count;
	unsigned int m_dist_count;
	unsigned int m_poison_count;
	CTime m_start_time;
	CTime m_end_time;
};