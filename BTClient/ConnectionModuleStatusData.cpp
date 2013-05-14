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
}

//
//
//
void ConnectionModuleStatusData::ClearCounters()
{

	//v_host_cache.clear();
	//v_connected_host_ips.clear();

	//m_idle_socket_count=0;
	//m_connecting_socket_count=0;
	//m_connected_socket_count=0;

	//m_ping_count=0;
	//m_pong_count=0;
	//m_push_count=0;
	//m_query_count=0;
	//m_query_hit_count=0;

}