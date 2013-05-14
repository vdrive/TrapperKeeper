#include "StdAfx.h"
#include "fileinfos.h"

FileInfos::FileInfos(void)
{
	Clear();
}

//
//
//
FileInfos::~FileInfos(void)
{
}

//
//
//
void FileInfos::Clear()
{
	v_files.clear();
}

//
//
//
int FileInfos::GetBufferLength()
{
	int len = 0;
	len+=sizeof(int);
	for(int i=0;i<(int)v_files.size();i++)
	{
		len+=v_files[i].GetBufferLength();
	}
	return len;
}

//
//
//
int FileInfos::WriteToBuffer(char* buf)
{
	int i;
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((int *)ptr)=(int)v_files.size();
	ptr+=sizeof(int);

	for(i=0;i<(int)v_files.size();i++)
	{
		ptr+=v_files[i].WriteToBuffer(ptr);
	}
	return (int)(ptr-buf);
}

//
//
//
int FileInfos::ReadFromBuffer(char* buf)
{
	int i;
	int len;
	Clear();
	char *ptr=buf;

	len=*((int *)ptr);
	ptr+=sizeof(int);

	for(i=0;i<len;i++)
	{
		FileInfo file;
		ptr += file.ReadFromBuffer(ptr);
		v_files.push_back(file);
	}
	return (int)(ptr-buf);
}