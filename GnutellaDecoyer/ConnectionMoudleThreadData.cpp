// ConnectionModuleThreadData.cpp

#include "stdafx.h"
#include "ConnectionModuleThreadData.h"

//
//
//
ConnectionModuleThreadData::ConnectionModuleThreadData()
{
//	memset(this,0,sizeof(ConnectionModuleThreadData));

	memset(m_reserved_events,0,sizeof(m_reserved_events));
	p_status_data=NULL;
	p_connect_hosts=NULL;
	p_compressed_qrp_table=NULL;
	p_connection_manager=NULL;
	vp_share_files=NULL;
}
