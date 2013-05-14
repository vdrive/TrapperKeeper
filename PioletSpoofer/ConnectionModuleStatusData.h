// ConnectionModuleStatusData.h
#pragma once

//#include "GnutellaHost.h"

#include "ProjectStatus.h"
#include "ProjectKeywords.h"

class ConnectionModule;

class ConnectionModuleStatusData
{
public:
	// Public Member Functions
	ConnectionModuleStatusData();
	~ConnectionModuleStatusData();
	void Clear();
	void ClearCounters();

//	void UpdateProjects(vector<ProjectKeywords> &keywords);

	// Public Data Members
//	unsigned int m_mod;
//	ConnectionModule *p_mod;

//	unsigned int m_idle_socket_count;
//	unsigned int m_connecting_socket_count;
//	unsigned int m_connected_socket_count;

//	vector<GnutellaHost> v_host_cache;

//	vector<unsigned int> v_connected_host_ips;
/*
	unsigned int m_ping_count;
	unsigned int m_pong_count;
	unsigned int m_push_count;
	unsigned int m_query_count;
	unsigned int m_query_hit_count;
*/
	vector<ProjectStatus> v_project_status;

//	vector<int> v_searcher_socket_query_counts;

	//unsigned int m_cached_query_hit_count;
};