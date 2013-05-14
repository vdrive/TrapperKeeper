// VirtualProjectStatusListCtrlItem.h
#pragma once

#include "ProjectStatus.h"

class VirtualProjectStatusListCtrlItem
{
public:
	// Public Member Functions
	VirtualProjectStatusListCtrlItem();
	~VirtualProjectStatusListCtrlItem();
	void Clear();
	void ClearCounters();

	bool operator<(VirtualProjectStatusListCtrlItem &item);

	void IncrementTrackCounters(ProjectStatus &status);
	void IncrementFakeTrackCounters(ProjectStatus &status);

	// Public Data Members
	string m_project_name;
	unsigned int m_query_hit_count;
	vector<unsigned int> v_track_fake_query_hit_count;
	vector<unsigned int> v_track_query_hit_count;

	unsigned int m_fake_query_hit_count;
	CTime m_last_update_time;
	bool m_has_been_written_to_db;
};