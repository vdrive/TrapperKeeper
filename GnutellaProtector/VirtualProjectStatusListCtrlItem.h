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

	// Public Data Members
	string m_project_name;
	unsigned int m_query_count;
//	unsigned int m_query_hit_count;
	vector<unsigned int> v_track_query_count;
//	vector<unsigned int> v_track_query_hit_count;

	unsigned int m_spoof_count;
	unsigned int m_dist_count;
	unsigned int m_poison_count;
};