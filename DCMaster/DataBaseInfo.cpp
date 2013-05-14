#include "StdAfx.h"
#include "databaseinfo.h"

DataBaseInfo::DataBaseInfo(void)
{
	Clear();
}

//
//
//
DataBaseInfo::~DataBaseInfo(void)
{
	Clear();
}

//
//
//
DataBaseInfo::DataBaseInfo(const DataBaseInfo &entry)
{
	m_db_host = entry.m_db_host;
	m_db_user = entry.m_db_user;
	m_db_password = entry.m_db_password;
}

//
//
//
DataBaseInfo& DataBaseInfo::operator=(const DataBaseInfo &entry)
{
	m_db_host = entry.m_db_host;
	m_db_user = entry.m_db_user;
	m_db_password = entry.m_db_password;	
	return *this;
}

//
//
//
void DataBaseInfo::Clear()
{
	m_db_host.erase();
	m_db_user.erase();
	m_db_password.erase();
}

//
//
//
int DataBaseInfo::GetBufferLength()
{

	int len=0;
	len+=(int)m_db_host.size()+1;	// +1 for NULL
	len+=(int)m_db_user.size()+1;
	len+=(int)m_db_password.size()+1;

	return len;
}

//
//
//
int DataBaseInfo::WriteToBuffer(char *buf)
{
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	strcpy(ptr,m_db_host.c_str());
	ptr+=m_db_host.size()+1;	// +1 for NULL

	strcpy(ptr,m_db_user.c_str());
	ptr+=m_db_user.size()+1;	// +1 for NULL

	strcpy(ptr,m_db_password.c_str());
	ptr+=m_db_password.size()+1;	// +1 for NULL

	return GetBufferLength();
}

//
// Returns buffer length on read
//
int DataBaseInfo::ReadFromBuffer(char *buf)
{
	Clear();

	char *ptr=buf;

	m_db_host=ptr;
	ptr+=m_db_host.size()+1;

	m_db_user=ptr;
	ptr+=m_db_user.size()+1;

	m_db_password=ptr;
	ptr+=m_db_password.size()+1;
	return GetBufferLength();
}

