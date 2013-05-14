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
	v_supply_data.clear();
}
