// ConnectionModuleThreadData.cpp

#include "stdafx.h"
#include "ConnectionModuleThreadData.h"

//
//
//
ConnectionModuleThreadData::ConnectionModuleThreadData()
{
	memset(m_reserved_events,0,sizeof(m_reserved_events));

	p_keywords=NULL;
	p_status_data=NULL;
}
