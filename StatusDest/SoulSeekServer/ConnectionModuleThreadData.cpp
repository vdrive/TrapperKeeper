// ConnectionModuleThreadData.cpp

#include "stdafx.h"
#include "ConnectionModuleThreadData.h"

//
//
//
ConnectionModuleThreadData::ConnectionModuleThreadData()
{
	Clear();
}

//
//
//
ConnectionModuleThreadData::~ConnectionModuleThreadData()
{
}

//
//
//
void ConnectionModuleThreadData::Clear()
{
	p_critical_section=NULL;
	memset(m_reserved_events,0,sizeof(m_reserved_events));
}