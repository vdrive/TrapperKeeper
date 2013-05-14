// VirtualProjectStatusListCtrlItem.cpp

#include "stdafx.h"
#include "VirtualProjectStatusListCtrlItem.h"

//
//
//
VirtualProjectStatusListCtrlItem::VirtualProjectStatusListCtrlItem()
{
	Clear();
}

//
//
//
VirtualProjectStatusListCtrlItem::~VirtualProjectStatusListCtrlItem()
{
}

//
//
//
void VirtualProjectStatusListCtrlItem::Clear()
{
	m_project_name.erase();
	
	ClearCounters();
}

//
//
//
void VirtualProjectStatusListCtrlItem::ClearCounters()
{
	m_query_count=0;
	m_query_hit_count=0;
	v_track_query_count.clear();
	v_track_query_hit_count.clear();

	m_spoof_count=0;
	m_dist_count=0;
	m_poison_count=0;
}

//
//
//
bool VirtualProjectStatusListCtrlItem::operator<(VirtualProjectStatusListCtrlItem &item)
{
	if(strcmp(m_project_name.c_str(),item.m_project_name.c_str())<0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
void VirtualProjectStatusListCtrlItem::IncrementTrackCounters(ProjectStatus &status)
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

	// Track Query Hits
	while(status.v_query_hit_counts.size()>v_track_query_hit_count.size())
	{
		v_track_query_hit_count.push_back(0);
	}

	for(i=0;i<status.v_query_hit_counts.size();i++)
	{
		v_track_query_hit_count[i]+=status.v_query_hit_counts[i];
	}
}