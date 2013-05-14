#include "StdAfx.h"
#include "fileinfo.h"

FileInfo::FileInfo(void)
{
	Clear();
}

//
//
//
FileInfo::~FileInfo(void)
{
}

//
//
//
void FileInfo::Clear()
{
	m_creation_date = CTime::GetCurrentTime();
	m_file_size=0;
	m_filename.Empty();
	m_path.Empty();
}

//
//
//
int FileInfo::GetBufferLength()
{
	int len=0;
	len += sizeof(CTime);
	len += sizeof(UINT);
	len += m_filename.GetLength()+1;
	len += m_path.GetLength()+1;

	return len;
}

//
//
//
int FileInfo::ReadFromBuffer(char* buf)
{
	Clear();
	char *ptr=buf;

	m_creation_date = *((CTime *)ptr);
	ptr+=sizeof(CTime);
	m_file_size = *((UINT *)ptr);
	ptr+=sizeof(UINT);
    
	m_filename=ptr;
	ptr+=m_filename.GetLength()+1;

	m_path = ptr;
	ptr+=m_path.GetLength()+1;
	return GetBufferLength();
}

//
//
//
int FileInfo::WriteToBuffer(char* buf)
{
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((CTime*)ptr)=m_creation_date;
	ptr+=sizeof(CTime);
	*((UINT*)ptr)=m_file_size;
	ptr+=sizeof(UINT);

	strcpy(ptr,m_filename);
	ptr+=m_filename.GetLength()+1;	// +1 for NULL

	strcpy(ptr,m_path);
	ptr+=m_path.GetLength()+1;	// +1 for NULL

	return GetBufferLength();
}