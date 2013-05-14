// Query.cpp

#include "stdafx.h"
#include "Query.h"

//
//
//
Query::Query()
{
	Clear();
}

//
//
//
Query::~Query()
{
}

//
//
//
void Query::Clear()
{
	//memset(&m_guid,0,sizeof(GUID));
	m_query.erase();
	m_project.erase();
	m_spoofs_sent=0;
//	m_dists_sent=0;
//	m_hops=0;
	//m_ttl=0;
	m_ip=0;
	m_track=0;
	m_timestamp = CTime::GetCurrentTime();
}

//
//
//
int Query::GetBufferLength()
{
	int len=0;
	
//	len+=sizeof(m_dists_sent);
//	len+=sizeof(m_guid);
//	len+=sizeof(m_hops);
	len+=sizeof(m_ip);
	len+=sizeof(m_spoofs_sent);
	len+=sizeof(m_timestamp);
	len+=sizeof(m_track);
//	len+=sizeof(m_ttl);
	len+=(int)m_query.size()+1;
	len+=(int)m_project.size()+1;
	
	return len;
}

//
//
//
int Query::WriteToBuffer(char *buf)
{
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;
/*
	*((UINT*)ptr)=m_dists_sent;
	ptr+=sizeof(UINT);

	*((GUID*)ptr)=m_guid;
	ptr+=sizeof(GUID);

	*((unsigned char*)ptr)=m_hops;
	ptr+=sizeof(unsigned char);
*/
	*((UINT*)ptr)=m_ip;
	ptr+=sizeof(UINT);

	*((UINT*)ptr)=m_spoofs_sent;
	ptr+=sizeof(UINT);

	*((CTime*)ptr)=m_timestamp;
	ptr+=sizeof(CTime);

	*((int*)ptr)=m_track;
	ptr+=sizeof(int);

//	*((unsigned char*)ptr)=m_ttl;
//	ptr+=sizeof(unsigned char);

    strcpy(ptr,m_query.c_str());
	ptr+=(int)m_query.size()+1;	// +1 for NULL

	strcpy(ptr, m_project.c_str());
	ptr+=(int)m_project.size()+1; //+1 for NULL

	return GetBufferLength();
}

//
// Returns buffer length on read
//
int Query::ReadFromBuffer(char *buf)
{
	Clear();

	char *ptr=buf;
/*
	m_dists_sent = *((UINT *)ptr);
	ptr+=sizeof(UINT);

	m_guid = *((GUID *)ptr);
	ptr+=sizeof(GUID);

	m_hops = *((unsigned char *)ptr);
	ptr+=sizeof(unsigned char);
*/
	m_ip = *((UINT *)ptr);
	ptr+=sizeof(UINT);

	m_spoofs_sent = *((UINT *)ptr);
	ptr+=sizeof(UINT);

	m_timestamp = *((CTime *)ptr);
	ptr+=sizeof(CTime);

	m_track = *((int *)ptr);
	ptr+=sizeof(int);

//	m_ttl = *((unsigned char *)ptr);
//	ptr+=sizeof(unsigned char);

	m_query=ptr;
	ptr+=(int)m_query.size()+1;

	m_project=ptr;
	ptr+=(int)m_project.size()+1;

	return GetBufferLength();
}
