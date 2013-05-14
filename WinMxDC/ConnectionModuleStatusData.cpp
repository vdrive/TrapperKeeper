// ConnectionModuleStatusData.cpp

#include "stdafx.h"
#include "ConnectionModuleStatusData.h"

//
//
//
ConnectionModuleStatusData::ConnectionModuleStatusData()
{
	Clear();
}

//
//
//
ConnectionModuleStatusData::~ConnectionModuleStatusData()
{
}

//
//
//
void ConnectionModuleStatusData::Clear()
{
	ClearCounters();
	
	m_mod=0;
	p_mod=NULL;

	v_project_status.clear();
}

//
//
//
void ConnectionModuleStatusData::ClearCounters()
{

	v_connected_host_ips.clear();
	v_other_hosts.clear();

	m_idle_socket_count=0;
	m_connecting_socket_count=0;
	m_connected_socket_count=0;

	m_ping_count=0;
	m_pong_count=0;
	m_push_count=0;
	m_query_count=0;
	m_query_hit_count=0;

	for(UINT i=0;i<v_project_status.size();i++)
	{
		v_project_status[i].ClearCounters();
	}
}
