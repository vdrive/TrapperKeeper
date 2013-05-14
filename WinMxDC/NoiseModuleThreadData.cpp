// NoiseModuleThreadData.cpp

#include "stdafx.h"
#include "NoiseModuleThreadData.h"

//
//
//
NoiseModuleThreadData::NoiseModuleThreadData()
{
//	memset(this,0,sizeof(NoiseModuleThreadData));

	p_incoming_connection_socket_handles=NULL;
	memset(m_reserved_events,0,sizeof(m_reserved_events));
	p_status_data=NULL;

}
