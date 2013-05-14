#include "StdAfx.h"
#include ".\fileinfo.h"

FileInfo::FileInfo(void)
{
}

FileInfo::~FileInfo(void)
{
}
//
///
//
void FileInfo::Clear()
{
	m_length = 0;
	m_md5.clear();
	m_pathname.clear();
}
	
//
//
//
int FileInfo::GetBufferLength()
{
	int len = 0;

	len += sizeof(m_length);
	len += (int)m_md5.size()+1;
	len += (int)m_pathname.size()+1;

	return len;	
}
	
//
//
//
int FileInfo::WriteToBuffer(char *buf)
{
	int i;
	int buf_length = GetBufferLength();

	memset(buf, 0, buf_length);

	char * ptr =  buf;

	*((unsigned int *)ptr)=m_length;
	ptr+=sizeof(unsigned int);

	strcpy(ptr, m_md5.c_str());
	ptr+=m_md5.size()+1;	//+1 for NULL

	strcpy(ptr, m_pathname.c_str());
	ptr+=m_pathname.size()+1;	//+1 for NULL

	return buf_length;
}

//
//
//
int FileInfo::ReadFromBuffer(char *buf)
{
	int i;

	Clear();

	char *ptr = buf;
	char *bufbegin = buf;
		
	m_length=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_md5 = ptr;
	ptr+=m_md5.size()+1;

	m_pathname = ptr;
	ptr+=m_pathname.size()+1;

	return (int)(ptr-bufbegin);
}