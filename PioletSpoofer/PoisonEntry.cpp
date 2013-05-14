#include "StdAfx.h"
#include "poisonentry.h"

PoisonEntry::PoisonEntry(void)
{
	Clear();
	
}

PoisonEntry::~PoisonEntry(void)
{
}

//
//
//
PoisonEntry* PoisonEntry::operator=(const PoisonEntry &entry)
{
	Clear();
	m_filename=entry.m_filename;
	m_filesize=entry.m_filesize;
	m_md5 = entry.m_md5;
	m_timestamp = entry.m_timestamp;
	m_requester_ip=entry.m_requester_ip;
	m_request_port=entry.m_request_port;
	
	return this;
}

//
//
//
void PoisonEntry::Clear()
{
	m_filename.erase();
	m_md5.erase();
	m_filesize=0;
	m_timestamp = CTime::GetCurrentTime();
	m_requester_ip.erase();
	m_request_port=0;
}

//
//
//
int PoisonEntry::GetBufferLength()
{
	int len=0;
	
	len += sizeof(m_filesize);
	len+=(int)m_filename.size()+1;	// +1 for NULL
	len+=(int)m_md5.size()+1;	// +1 for NULL
	len+=sizeof(m_timestamp);
	len+=(int)m_requester_ip.size()+1;
	len+=sizeof(m_request_port);
	return len;
}

//
//
//
int PoisonEntry::WriteToBuffer(char *buf)
{
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((UINT*)ptr)=m_filesize;
	ptr+=sizeof(UINT);

	strcpy(ptr,m_filename.c_str());
	ptr+=m_filename.size()+1;	// +1 for NULL

	strcpy(ptr,m_md5.c_str());
	ptr+=m_md5.size()+1;	// +1 for NULL

	*((CTime*)ptr)=m_timestamp;
	ptr+=sizeof(m_timestamp);

	strcpy(ptr,m_requester_ip.c_str());
	ptr+=m_requester_ip.size()+1;	// +1 for NULL
	*((int*)ptr)=m_request_port;
	ptr+=sizeof(int);


	return GetBufferLength();
}

//
// Returns buffer length on read
//
int PoisonEntry::ReadFromBuffer(char *buf)
{
	Clear();

	char *ptr=buf;

	m_filesize = *((UINT *)ptr);
	ptr+=sizeof(UINT);

	m_filename=ptr;
	ptr+=m_filename.size()+1;

	m_md5=ptr;
	ptr+=m_md5.size()+1;

	m_timestamp = *((CTime*)ptr);

	m_requester_ip=ptr;
	ptr+=m_requester_ip.size()+1;

	m_request_port = *((int *)ptr);
	ptr+=sizeof(int);

	return GetBufferLength();
}