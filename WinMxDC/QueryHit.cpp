// QueryHit.cpp

#include "stdafx.h"
#include "QueryHit.h"

//
//
//
QueryHit::QueryHit()
{
	Clear();
}

//
//
//
QueryHit::QueryHit(const QueryHit &hit)
{
	Clear();
	*this = hit;
}


//
//
//
QueryHit::~QueryHit()
{
}

//
//
//
QueryHit& QueryHit::operator=(const QueryHit &hit)
{
	Clear();
	m_search_id=hit.m_search_id;
	m_port=hit.m_port;
	m_ip=hit.m_ip;
	m_file_size=hit.m_file_size;
	m_track=hit.m_track;
	m_timestamp = hit.m_timestamp;
	memcpy(&m_hash, hit.m_hash, sizeof(m_hash));
	m_filename = hit.m_filename;
	m_project = hit.m_project;
	m_project_id=hit.m_project_id;
	m_bitrate = hit.m_bitrate;
	m_sample_frequency = hit.m_sample_frequency;
	m_duration = hit.m_duration;
	m_folder_name = hit.m_folder_name;
	return *this;
}

//
//
//
void QueryHit::Clear()
{
	memset(&m_hash,0,sizeof(m_hash));
	m_port=0;
	m_ip=0;
	m_file_size=0;
	m_track=0;
	m_timestamp = CTime::GetCurrentTime();
	m_filename.Empty();
	m_project.Empty();
	m_project_id=0;
	m_bitrate = 0;
	m_sample_frequency = 0;
	m_duration = 0;
	m_folder_name.Empty();
}

//
//
//
int QueryHit::GetBufferLength()
{
	int len=0;
	
	len+=sizeof(m_search_id);
	len+=sizeof(m_port);
	len+=sizeof(m_ip);
	len+=sizeof(m_file_size);
	len+=sizeof(m_track);
	len+=sizeof(m_hash);
	len+=sizeof(m_timestamp);
	len+=m_project.GetLength()+1;
	len+=m_filename.GetLength()+1;
	len += sizeof(m_project_id);
	len += sizeof(m_bitrate);
	len+=sizeof(m_sample_frequency);
	len+=sizeof(m_duration);
	len+=sizeof(m_media_type);
	
	return len;
}

//
//
//
int QueryHit::WriteToBuffer(char *buf)
{
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((unsigned short*)ptr)=m_search_id;
	ptr+=sizeof(unsigned short);

	*((unsigned short*)ptr)=m_port;
	ptr+=sizeof(unsigned short);

	*((unsigned int*)ptr)=m_ip;
	ptr+=sizeof(unsigned int);

	*((unsigned int*)ptr)=m_file_size;
	ptr+=sizeof(unsigned int);

	*((unsigned short*)ptr)=m_track;
	ptr+=sizeof(unsigned short);

	*((CTime*)ptr)=m_timestamp;
	ptr+=sizeof(CTime);

	memcpy(ptr, m_hash, sizeof(m_hash));
	ptr+=sizeof(m_hash);

	strcpy(ptr, m_filename);
	ptr+=m_filename.GetLength()+1; //+1 for NULL

	strcpy(ptr, m_project);
	ptr+=m_project.GetLength()+1; //+1 for NULL

	*((unsigned int*)ptr)=m_project_id;
	ptr+=sizeof(unsigned int);

	*((WORD*)ptr)=m_bitrate;
	ptr+=sizeof(WORD);
	*((int*)ptr)=m_sample_frequency;
	ptr+=sizeof(int);
	*((int*)ptr)=m_duration;
	ptr+=sizeof(int);
	*((WINMXMediaType*)ptr)=m_media_type;
	ptr+=sizeof(WINMXMediaType);



	return GetBufferLength();
}

//
// Returns buffer length on read
//
int QueryHit::ReadFromBuffer(char *buf)
{
	Clear();

	char *ptr=buf;

	m_search_id = *((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);

	m_port = *((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);

	m_ip = *((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_file_size = *((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_track = *((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);

	m_timestamp = *((CTime *)ptr);
	ptr+=sizeof(CTime);

	memcpy(m_hash, ptr, sizeof(m_hash));
	ptr+=sizeof(m_hash);

	m_filename=ptr;
	ptr+=m_filename.GetLength()+1;

	m_project=ptr;
	ptr+=m_project.GetLength()+1;

	m_project_id = *((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_bitrate = *((WORD*)ptr);
	ptr+=sizeof(WORD);
	m_sample_frequency = *((int *)ptr);
	ptr+=sizeof(int);
	m_duration = *((int *)ptr);
	ptr+=sizeof(int);
	m_media_type = *((WINMXMediaType*)ptr);
	ptr+=sizeof(WINMXMediaType);

	return GetBufferLength();
}
