// QueryKeywords.cpp

#include "stdafx.h"
#include "QueryKeywords.h"

//
//
//
QueryKeywords::QueryKeywords()
{
	Clear();
}

//
//
//
QueryKeywords::~QueryKeywords()
{
}

//
//
//
void QueryKeywords::Clear()
{
/*
	// Public Data Members
	vector<string> v_keywords;
	vector<string> v_exact_keywords;
	vector<string> v_killwords;
	vector<string> v_exact_killwords;
*/
	v_keywords.clear();
	v_exact_keywords.clear();
	v_killwords.clear();
	v_exact_killwords.clear();
}

//
//
//
int QueryKeywords::GetBufferLength()
{
/*
	// Public Data Members
	vector<string> v_keywords;
	vector<string> v_exact_keywords;
	vector<string> v_killwords;
	vector<string> v_exact_killwords;
*/
	UINT i;
	int len=0;

	len+=sizeof(unsigned int);	// number of keywords
	for(i=0;i<v_keywords.size();i++)
	{
		len+=sizeof(unsigned int);
		len+=(int)v_keywords[i].keyword.size()+1;	// +1 for the NULL
	}

	len+=sizeof(unsigned int);	// number of exact keywords
	for(i=0;i<v_exact_keywords.size();i++)
	{
		len+=sizeof(unsigned int);
		len+=(int)v_exact_keywords[i].keyword.size()+1;	// +1 for the NULL
	}

	len+=sizeof(unsigned int);	// number of kill words
	for(i=0;i<v_killwords.size();i++)
	{
		len+=sizeof(unsigned int);
		len+=(int)v_killwords[i].keyword.size()+1;	// +1 for the NULL
	}

	len+=sizeof(unsigned int);	// number of exact kill words
	for(i=0;i<v_exact_killwords.size();i++)
	{
		len+=sizeof(unsigned int);
		len+=(int)v_exact_killwords[i].keyword.size()+1;	// +1 for the NULL
	}

	return len;
}

//
//
//
int QueryKeywords::WriteToBuffer(char *buf)
{
/*
	// Public Data Members
	vector<string> v_keywords;
	vector<string> v_exact_keywords;
	vector<string> v_killwords;
	vector<string> v_exact_killwords;
*/
	int i;

	// Clear the buffer
	memset(buf,0,GetBufferLength());
	
	char *ptr=buf;

	*((int *)ptr)=(int)v_keywords.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_keywords.size();i++)
	{
		*((unsigned int*)ptr)=v_keywords[i].weight;
		ptr+=sizeof(unsigned int);
		strcpy(ptr,v_keywords[i].keyword.c_str());
		ptr+=(int)v_keywords[i].keyword.size()+1;	// +1 for NULL
	}

	*((int *)ptr)=(int)v_exact_keywords.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_exact_keywords.size();i++)
	{
		*((unsigned int*)ptr)=v_exact_keywords[i].weight;
		ptr+=sizeof(unsigned int);
		strcpy(ptr,v_exact_keywords[i].keyword.c_str());
		ptr+=(int)v_exact_keywords[i].keyword.size()+1;	// +1 for NULL
	}

	*((int *)ptr)=(int)v_killwords.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_killwords.size();i++)
	{
		*((unsigned int*)ptr)=v_killwords[i].weight;
		ptr+=sizeof(unsigned int);
		strcpy(ptr,v_killwords[i].keyword.c_str());
		ptr+=(int)v_killwords[i].keyword.size()+1;	// +1 for NULL
	}

	*((int *)ptr)=(int)v_exact_killwords.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_exact_killwords.size();i++)
	{
		*((unsigned int*)ptr)=v_exact_killwords[i].weight;
		ptr+=sizeof(unsigned int);
		strcpy(ptr,v_exact_killwords[i].keyword.c_str());
		ptr+=(int)v_exact_killwords[i].keyword.size()+1;	// +1 for NULL
	}

	return GetBufferLength();
}

//
//
//
int QueryKeywords::ReadFromBuffer(char *buf)
{
/*
	// Public Data Members
	vector<string> v_keywords;
	vector<string> v_exact_keywords;
	vector<string> v_killwords;
	vector<string> v_exact_killwords;
*/
	int i;
	int len;
	
	Clear();

	char *ptr=buf;

	len=*((int *)ptr);
	ptr+=sizeof(int);
	for(i=0;i<len;i++)
	{
		QueryKeyword keyword;
		keyword.weight = *((unsigned int*)ptr);
		ptr+=sizeof(unsigned int);
		keyword.keyword = ptr;
		v_keywords.push_back(keyword);
		ptr+=strlen(ptr)+1;	// +1 for NULL
	}

	len=*((int *)ptr);
	ptr+=sizeof(int);
	for(i=0;i<len;i++)
	{
		QueryKeyword keyword;
		keyword.weight = *((unsigned int*)ptr);
		ptr+=sizeof(unsigned int);
		keyword.keyword = ptr;
		v_exact_keywords.push_back(keyword);
		ptr+=strlen(ptr)+1;	// +1 for NULL
	}

	len=*((int *)ptr);
	ptr+=sizeof(int);
	for(i=0;i<len;i++)
	{
		QueryKeyword keyword;
		keyword.weight = *((unsigned int*)ptr);
		ptr+=sizeof(unsigned int);
		keyword.keyword = ptr;
		v_killwords.push_back(keyword);
		ptr+=strlen(ptr)+1;	// +1 for NULL
	}

	len=*((int *)ptr);
	ptr+=sizeof(int);
	for(i=0;i<len;i++)
	{
		QueryKeyword keyword;
		keyword.weight = *((unsigned int*)ptr);
		ptr+=sizeof(unsigned int);
		keyword.keyword = ptr;
		v_exact_killwords.push_back(keyword);
		ptr+=strlen(ptr)+1;	// +1 for NULL
	}

	return GetBufferLength();
}