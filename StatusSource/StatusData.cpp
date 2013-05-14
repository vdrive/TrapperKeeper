// StatusData.cpp

#include "stdafx.h"
#include "StatusData.h"
#include "DllInterface.h"

//
//
//
StatusData::StatusData()
{
	Clear();
}

//
//
//
StatusData::StatusData(StatusData* status_data)
{
	Clear();
	m_free_bytes = status_data->m_free_bytes;
	m_kazaa_count = status_data->m_kazaa_count;
	m_percent_processor_usage = status_data->m_percent_processor_usage;
	m_max_kazaa = status_data->m_max_kazaa;
	m_shared_file_count = status_data->m_shared_file_count;
	m_total_used_bandwidth = status_data->m_total_used_bandwidth;
	m_version = status_data->m_version;
	v_kazaa_mems = status_data->v_kazaa_mems;
	m_movies_shared = status_data->m_movies_shared;
	m_music_shared = status_data->m_music_shared;
	m_swarmer_shared = status_data->m_swarmer_shared;
	m_received_per_sec = status_data->m_received_per_sec;
	m_sent_per_sec = status_data->m_sent_per_sec;
	m_trapper_keeper_mem_usage = status_data->m_trapper_keeper_mem_usage;
	m_committed_memory = status_data->m_committed_memory;
}

//
//
//
StatusData& StatusData::operator=(const StatusData* status_data)	// assignment operator
{
	Clear();
	m_free_bytes = status_data->m_free_bytes;
	m_kazaa_count = status_data->m_kazaa_count;
	m_percent_processor_usage = status_data->m_percent_processor_usage;
	m_max_kazaa = status_data->m_max_kazaa;
	m_shared_file_count = status_data->m_shared_file_count;
	m_total_used_bandwidth = status_data->m_total_used_bandwidth;
	m_version = status_data->m_version;
	v_kazaa_mems = status_data->v_kazaa_mems;
	m_movies_shared = status_data->m_movies_shared;
	m_music_shared = status_data->m_music_shared;
	m_swarmer_shared = status_data->m_swarmer_shared;
	m_received_per_sec = status_data->m_received_per_sec;
	m_sent_per_sec = status_data->m_sent_per_sec;
	m_trapper_keeper_mem_usage = status_data->m_trapper_keeper_mem_usage;
	m_committed_memory = status_data->m_committed_memory;
	return *this;
}

//
//
//
void StatusData::Clear()
{
	m_version=DllInterface::m_app_id.m_version;
	m_percent_processor_usage=0;
//	m_total_available_bandwidth=0;
	m_total_used_bandwidth=0;
	m_kazaa_count=0;
	m_shared_file_count=0;
	m_max_kazaa=0;
	m_movies_shared=0;
	m_music_shared=0;
	m_swarmer_shared=0;
	v_kazaa_mems.clear();
	m_received_per_sec = 0;
	m_sent_per_sec = 0;
	m_trapper_keeper_mem_usage = 0;
	m_committed_memory = 0;
}

//
//
//
int StatusData::GetBufferLength()
{
	int len=0;
	len += sizeof(UINT);	//m_version
	len += sizeof(UINT);	//m_percent_processor_usage
	len += sizeof(UINT);	//m_total_used_bandwidth
	len += sizeof(UINT);	//m_kazaa_count
	len += sizeof(UINT);	//m_shared_file_count
	len += sizeof(UINT);	//m_max_kazaa
	len += sizeof(float);	//m_free_bytes
	len += sizeof(UINT)*(int)v_kazaa_mems.size();

	//new version
	len += sizeof(UINT);	//m_movies_shared
	len += sizeof(UINT);	//m_music_shared
	len += sizeof(UINT);	//m_swarmer_shared

	//new new version
	len += sizeof(UINT);	//m_received_per_sec
	len += sizeof(UINT);	//m_sent_per_sec
	len += sizeof(UINT);	//m_trapper_keeper_mem_usage
	len += sizeof(UINT);	//m_committed_memory

	return len;
}

//
//
//
int StatusData::WriteToBuffer(char *buf)
{
	UINT i;
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((UINT*)ptr)=m_version;
	ptr+=sizeof(UINT);
	*((UINT*)ptr)=m_percent_processor_usage;
	ptr+=sizeof(UINT);
	*((UINT*)ptr)=m_total_used_bandwidth;
	ptr+=sizeof(UINT);
	*((UINT*)ptr)=m_kazaa_count;
	ptr+=sizeof(UINT);
	*((UINT*)ptr)=m_shared_file_count;
	ptr+=sizeof(UINT);
	*((UINT*)ptr)=m_max_kazaa;
	ptr+=sizeof(UINT);
	*((float*)ptr)=m_free_bytes;
	ptr+=sizeof(float);
	
	for(i=0;i<m_kazaa_count;i++)
	{
		*((UINT *)ptr)=v_kazaa_mems[i];
		ptr+=sizeof(UINT);
	}

	*((UINT*)ptr)=m_movies_shared;
	ptr+=sizeof(UINT);
	*((UINT*)ptr)=m_music_shared;
	ptr+=sizeof(UINT);
	*((UINT*)ptr)=m_swarmer_shared;
	ptr+=sizeof(UINT);

	*((UINT*)ptr)=m_received_per_sec;
	ptr+=sizeof(UINT);
	*((UINT*)ptr)=m_sent_per_sec;
	ptr+=sizeof(UINT);
	*((UINT*)ptr)=m_trapper_keeper_mem_usage;
	ptr+=sizeof(UINT);	
	*((UINT*)ptr)=m_committed_memory;
	ptr+=sizeof(UINT);	

	return GetBufferLength();
}

//
// Returns buffer length on read
//
int StatusData::ReadFromBuffer(char *buf)
{
	UINT i;
	Clear();

	char *ptr=buf;

	m_version = *((UINT *)ptr);
	ptr+=sizeof(UINT);
	m_percent_processor_usage = *((UINT *)ptr);
	ptr+=sizeof(UINT);
	m_total_used_bandwidth = *((UINT *)ptr);
	ptr+=sizeof(UINT);
	m_kazaa_count = *((UINT *)ptr);
	ptr+=sizeof(UINT);
	m_shared_file_count = *((UINT *)ptr);
	ptr+=sizeof(UINT);
	m_max_kazaa = *((UINT *)ptr);
	ptr+=sizeof(UINT);
	m_free_bytes = *((float *)ptr);
	ptr+=sizeof(float);

	for(i=0;i<m_kazaa_count;i++)
	{
		v_kazaa_mems.push_back(*((UINT *)ptr));
		ptr+=sizeof(UINT);
	}

	if(m_version > 0x0000000A)
	{
		m_movies_shared = *((UINT*)ptr);
		ptr+=sizeof(UINT);
		m_music_shared = *((UINT*)ptr);
		ptr+=sizeof(UINT);
		m_swarmer_shared = *((UINT*)ptr);
		ptr+=sizeof(UINT);
	}
	if(m_version > 0x0000000C)
	{
		m_received_per_sec = *((UINT*)ptr);
		ptr+=sizeof(UINT);
		m_sent_per_sec = *((UINT*)ptr);
		ptr+=sizeof(UINT);
		m_trapper_keeper_mem_usage = *((UINT*)ptr);
		ptr+=sizeof(UINT);
		m_committed_memory = *((UINT*)ptr);
		ptr+=sizeof(UINT);
	}

	return GetBufferLength();
}