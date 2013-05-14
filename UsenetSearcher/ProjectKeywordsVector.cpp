#include "StdAfx.h"
#include "ProjectKeywordsVector.h"

ProjectKeywordsVector::ProjectKeywordsVector(void)
{
	Clear();
}

//
//
//
ProjectKeywordsVector::~ProjectKeywordsVector(void)
{
}

//
//
//
void ProjectKeywordsVector::Clear()
{
	v_projects.clear();
}

//
//
//
int ProjectKeywordsVector::GetBufferLength()
{
	int len=0;

	len+=sizeof(int);	// number of killwords
	for(int i=0;i<(int)v_projects.size();i++)
	{
		len+=(int)v_projects[i].GetBufferLength();
	}

	return len;
}

//
//
//
int ProjectKeywordsVector::WriteToBuffer(char *buf)
{
	int i;

	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((int *)ptr)=(int)v_projects.size();
	ptr+=sizeof(int);
	
	for(i=0;i<(int)v_projects.size();i++)
	{
		ptr+=v_projects[i].WriteToBuffer(ptr);
	}

	return GetBufferLength();
}

//
// Returns buffer length on read
//
int ProjectKeywordsVector::ReadFromBuffer(char *buf)
{
	int i;

	Clear();

	char *ptr=buf;

	int len=*((int *)ptr);
	ptr+=sizeof(int);

	for(i=0;i<len;i++)
	{
		ProjectKeywords word;
		ptr+=word.ReadFromBuffer(ptr);
		v_projects.push_back(word);
	}

	return GetBufferLength();
}