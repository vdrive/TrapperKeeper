// SupplyKeyword.cpp

#include "stdafx.h"
#include "SupplyKeyword.h"

//
//
//
SupplyKeyword::SupplyKeyword()
{
	Clear();
}

//
//
//
SupplyKeyword::SupplyKeyword(const SupplyKeyword &keyword)
{
	Clear();

	m_track=keyword.m_track;

	m_single = keyword.m_single;

	m_track_name = keyword.m_track_name;

	char *ptr=NULL;
	for(int i=0;i<(int)keyword.v_keywords.size();i++)
	{
		ptr=new char[strlen(keyword.v_keywords[i])+1];
		strcpy(ptr,keyword.v_keywords[i]);
		v_keywords.push_back(ptr);
	}
}

//
//
//
SupplyKeyword::~SupplyKeyword()
{
	Clear();
}

//
//
//
void SupplyKeyword::Clear()
{
	m_track=0;

	m_single = false;

	m_track_name.erase();

	for(int i=0;i<(int)v_keywords.size();i++)
	{
		delete [] v_keywords[i];
	}
	v_keywords.clear();
}

//
//
//
SupplyKeyword& SupplyKeyword::operator=(const SupplyKeyword &keyword)
{
	Clear();

	m_track=keyword.m_track;

	m_single = keyword.m_single;

	m_track_name = keyword.m_track_name;

	char *ptr=NULL;
	for(int i=0;i<(int)keyword.v_keywords.size();i++)
	{
		ptr=new char[strlen(keyword.v_keywords[i])+1];
		strcpy(ptr,keyword.v_keywords[i]);
		v_keywords.push_back(ptr);
	}
	
	return *this;
}

//
//
//
int SupplyKeyword::GetBufferLength()
{
	int len=0;

	len+=sizeof(bool);  //m_single

	len+=sizeof(unsigned int);	// m_track

	len+=(int)m_track_name.size()+1; //+1 for NULL

	len+=sizeof(unsigned int);	// # of keywords
	for(int i=0;i<(int)v_keywords.size();i++)
	{
		len+=(int)strlen(v_keywords[i])+1;	// +1 for NULL
	}

	return len;
}

//
//
//
int SupplyKeyword::WriteToBuffer(char *buf)
{
	char *ptr=buf;

	memset(ptr,0,GetBufferLength());

	*((bool *)ptr)=m_single;
	ptr+=sizeof(bool);

	*((unsigned int *)ptr)=m_track;
	ptr+=sizeof(unsigned int);

	strcpy(ptr, m_track_name.c_str());
	ptr+=m_track_name.size()+1; //+1 for NULL

	*((unsigned int *)ptr)=(int)v_keywords.size();
	ptr+=sizeof(unsigned int);

	for(int i=0;i<(int)v_keywords.size();i++)
	{
		strcpy(ptr,v_keywords[i]);
		ptr+=strlen(v_keywords[i])+1;	// +1 for NULL
	}

	return GetBufferLength();
}

//
//
//
int SupplyKeyword::ReadFromBuffer(char *buf)
{
	char *ptr=buf;

	m_single=*((bool *)ptr);
	ptr+=sizeof(bool);

	m_track=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_track_name = ptr;
	ptr+=m_track_name.size()+1; //+1 for NULL

	unsigned int num_keywords=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	for(UINT i=0;i<num_keywords;i++)
	{
		char *buffer=new char[strlen(ptr)+1];
		strcpy(buffer,ptr);
		v_keywords.push_back(buffer);	
		ptr+=strlen(ptr)+1;	// +1 for NULL
	}

	return GetBufferLength();
}