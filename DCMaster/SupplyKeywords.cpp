// SupplyKeywords.cpp

#include "stdafx.h"
#include "SupplyKeywords.h"

//
//
//
SupplyKeywords::SupplyKeywords()
{
	Clear();
}

//
//
//
SupplyKeywords::~SupplyKeywords()
{
}

//
//
//
void SupplyKeywords::Clear()
{
/*
	// Public Data Members
	unsigned int m_supply_size_threshold;
	vector<string> v_keywords;
	vector<string> v_killwords;
	vector<string> v_required_keywords;
*/
	m_supply_size_threshold=0;
	m_search_string.erase();
	v_killwords.clear();
	v_keywords.clear();
}

//
//
//
int SupplyKeywords::GetBufferLength()
{
/*
	// Public Data Members
	unsigned int m_supply_size_threshold;
	vector<string> v_keywords;
	vector<string> v_killwords;
	vector<string> v_required_keywords;
*/
	int i;
	int len=0;

	len+=sizeof(unsigned int);	// supply size threshold

	len+=(int)m_search_string.size()+1;	// +1 for NULL

	len+=sizeof(int);	// number of killwords
	for(i=0;i<(int)v_killwords.size();i++)
	{
		len+=(int)v_killwords[i].GetBufferLength();
//		len+=sizeof(unsigned int);
//		len+=v_killwords[i].keyword.size()+1;	// +1 for NULL
	}

	len+=sizeof(int);	// number of required keywords
	for(i=0;i<(int)v_keywords.size();i++)
	{
		len+=(int)v_keywords[i].GetBufferLength();
//		len+=sizeof(unsigned int);
//		len+=v_keywords[i].keyword.size()+1;	// +1 for NULL
	}

	return len;
}

//
//
//
int SupplyKeywords::WriteToBuffer(char *buf)
{
/*
	// Public Data Members
	unsigned int m_supply_size_threshold;
	vector<string> v_keywords;
	vector<string> v_killwords;
	vector<string> v_required_keywords;
*/
	int i;

	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	*((unsigned int *)ptr)=m_supply_size_threshold;
	ptr+=sizeof(unsigned int);

	strcpy(ptr,m_search_string.c_str());
	ptr+=m_search_string.size()+1;	// +1 for NULL

	*((int *)ptr)=(int)v_killwords.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_killwords.size();i++)
	{
		ptr+=v_killwords[i].WriteToBuffer(ptr);
/*
		*((unsigned int *)ptr)=v_killwords[i].m_track;
		ptr+=sizeof(unsigned int);
		strcpy(ptr,v_killwords[i].keyword.c_str());
		ptr+=v_killwords[i].keyword.size()+1;	// +1 for NULL
*/
	}

	*((int *)ptr)=(int)v_keywords.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_keywords.size();i++)
	{
		ptr+=v_keywords[i].WriteToBuffer(ptr);
/*
		*((unsigned int *)ptr)=v_keywords[i].track;
		ptr+=sizeof(unsigned int);
		strcpy(ptr,v_keywords[i].keyword.c_str());
		ptr+=v_keywords[i].keyword.size()+1;	// +1 for NULL
*/
	}

	return (int)(ptr-buf);
}

//
//
//
int SupplyKeywords::ReadFromBuffer(char *buf)
{
/*
	// Public Data Members
	unsigned int m_supply_size_threshold;
	vector<string> v_keywords;
	vector<string> v_killwords;
	vector<string> v_required_keywords;
*/
	int i;
	int len;

	Clear();

	char *ptr=buf;

	m_supply_size_threshold=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_search_string = ptr;
	ptr+=strlen(ptr)+1;

	len=*((int *)ptr);
	ptr+=sizeof(int);
	for(i=0;i<len;i++)
	{
		SupplyKeyword word;
		ptr+=word.ReadFromBuffer(ptr);
		v_killwords.push_back(word);
/*
		word.track=*((unsigned int *)ptr);
		ptr+=sizeof(unsigned int);
		word.keyword = ptr;
		v_killwords.push_back(word);
		ptr+=strlen(ptr)+1;	// +1 for NULL
*/
	}

	len=*((int *)ptr);
	ptr+=sizeof(int);
	for(i=0;i<len;i++)
	{
		SupplyKeyword word;
		ptr+=word.ReadFromBuffer(ptr);
		v_keywords.push_back(word);
/*
		word.track=*((unsigned int *)ptr);
		ptr+=sizeof(unsigned int);
		word.keyword = ptr;
		v_keywords.push_back(word);
		ptr+=strlen(ptr)+1;	// +1 for NULL
*/
	}

	return (int)(ptr-buf);
}