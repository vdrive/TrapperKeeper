// NoiseModuleThreadStatusData.cpp

#include "stdafx.h"
#include "NoiseModuleThreadStatusData.h"

//
//
//
NoiseModuleThreadStatusData::NoiseModuleThreadStatusData()
{
	Clear();
}

//
//
//
void NoiseModuleThreadStatusData::Clear()
{
//	memset(this,0,sizeof(NoiseModuleThreadStatusData));
	
	m_is_idle=true;

	m_filename.Empty();
	m_start=0;
	m_end=0;
	m_num_sent=0;

	m_mod=0;
	m_socket=0;

	m_remote_ip=0;

	m_start_time=0;

	m_client.Empty();

	m_num_bogus_connections=0;
	m_num_good_connections=0;
	m_total_size=0;
}