// ConnectionModuleThreadData.cpp

#include "stdafx.h"
#include "ConnectionModuleThreadData.h"

//
//
//
ConnectionModuleThreadData::ConnectionModuleThreadData()
{
	memset(m_reserved_events,0,sizeof(m_reserved_events));
	p_status_data=NULL;
	p_connect_hosts=NULL;
	p_project_supply_queries=NULL;
	p_keywords=NULL;
	//p_supply=NULL;
	m_reconnect_to_hosts = false;
	m_disconnect_to_hosts = false;
	p_file_sharing_manager=NULL;
}
