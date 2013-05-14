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
	m_num_peers=0;
	m_num_projects=0;
	m_num_supplies=0;
	m_hash_search_index=0;
	m_publicize_ack_received=0;
	m_userhash_to_publicize=0;
	m_queries_received=0;
	m_publish_received=0;
	m_spoofs_sent=0;
}
