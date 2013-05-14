#include "StdAfx.h"
#include ".\hashtable.h"

HashTable::HashTable(void)
{
		Clear();
}

//
//
//
HashTable::~HashTable(void)
{
}

//
//
//
void HashTable::Clear()
{
	m_sha1.Empty();
	m_filesize=0;
}

//
//
//
int HashTable::GetBufferLength()
{
	int len=sizeof(m_filesize); //m_filesize
	len+=(int)m_sha1.GetLength()+1;
	return len;
}

//
//
//
int HashTable::WriteToBuffer(char *buf)
{
	int i;
	// Clear buffer
	memset(buf,0,GetBufferLength());
	char *ptr=buf;
	*((UINT*)ptr)=m_filesize;
	ptr+=sizeof(UINT);

	strcpy(ptr,m_sha1);
	ptr+=m_sha1.GetLength()+1;	// +1 for NULL
	return GetBufferLength();
}

//
// Returns buffer length on read
//
int HashTable::ReadFromBuffer(char *buf)
{
	int i;
	Clear();
	char *ptr=buf;

	m_filesize = *((UINT *)ptr);
	ptr+=sizeof(UINT);

	m_sha1=ptr;
	return GetBufferLength();
}