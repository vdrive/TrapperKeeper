// ConnectionModuleThreadData.cpp

#include "stdafx.h"
#include "FTConnectionModuleThreadData.h"

//
//
//
FTConnectionModuleThreadData::FTConnectionModuleThreadData()
{
//	memset(this,0,sizeof(ConnectionModuleThreadData));

	memset(m_reserved_events,0,sizeof(m_reserved_events));

//	p_status_data=NULL;
//	p_keywords=NULL;
//	p_supply=NULL;
//	p_project_supply_queries=NULL;

//	p_max_host=NULL;
//	p_max_host_cache=NULL;
//	p_vendor_counts=NULL;
	m_port=0;
	m_file_length=0;
	m_md5.erase();
	m_num_idle_sockets=60;
	//m_socket=NULL;
	p_incoming_connection_socket_handles=NULL;
}
