// QueryHitResult.cpp

#include "stdafx.h"
#include "QueryHitResult.h"

//
//
//
QueryHitResult::QueryHitResult()
{
	Clear();
}

//
//
//
QueryHitResult::~QueryHitResult()
{
	Clear();
}

//
//
//
void QueryHitResult::Clear()
{
	m_file_index=0;
	m_file_size=0;
	m_file_name.erase();
	m_info.erase();
}

//
//
//
int QueryHitResult::GetBufferLength()
{
	int len=0;

	// File Index and File Size
	len+=sizeof(unsigned int)+sizeof(unsigned int);
	len+=(int)m_file_name.size()+1;	// +1 for NULL
	len+=(int)m_info.size()+1;		// +1 for NULL

	return len;
}

//
//
//
void QueryHitResult::WriteToBuffer(char *buf)
{
	char *ptr=buf;

	memset(ptr,0,GetBufferLength());

	*((unsigned int *)ptr)=m_file_index;
	ptr+=sizeof(unsigned int);
	*((unsigned int *)ptr)=m_file_size;
	ptr+=sizeof(unsigned int);

	strcpy(ptr,m_file_name.c_str());
	ptr+=m_file_name.size()+1;

	strcpy(ptr,m_info.c_str());
	ptr+=m_info.size()+1;
}
