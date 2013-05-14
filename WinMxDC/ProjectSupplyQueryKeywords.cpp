// ProjectSupplyQueryKeywords.cpp

#include "stdafx.h"
#include "ProjectSupplyQueryKeywords.h"

//
//
//
ProjectSupplyQueryKeywords::ProjectSupplyQueryKeywords()
{
	Clear();
}

//
//
//
ProjectSupplyQueryKeywords::ProjectSupplyQueryKeywords(const ProjectSupplyQueryKeywords &psqrk)
{
	
	UINT i;

	Clear();

	m_track=psqrk.m_track;

	for(i=0;i<psqrk.v_keywords.size();i++)
	{
		char *ptr=new char[strlen(psqrk.v_keywords[i])+1];
		strcpy(ptr,psqrk.v_keywords[i]);
		v_keywords.push_back(ptr);
	}
}

//
//
//
ProjectSupplyQueryKeywords::~ProjectSupplyQueryKeywords()
{
	UINT i;

	for(i=0;i<v_keywords.size();i++)
	{
		delete [] v_keywords[i];
	}
	v_keywords.clear();
}

//
//
//
void ProjectSupplyQueryKeywords::Clear()
{
	UINT i;

	m_track=0;

	for(i=0;i<v_keywords.size();i++)
	{
		delete [] v_keywords[i];
	}
	v_keywords.clear();
}

//
//
//
ProjectSupplyQueryKeywords& ProjectSupplyQueryKeywords::operator=(const ProjectSupplyQueryKeywords &psq)
{
	UINT i;

	Clear();

	m_track=psq.m_track;

	for(i=0;i<psq.v_keywords.size();i++)
	{
		char *ptr=new char[strlen(psq.v_keywords[i])+1];
		strcpy(ptr,psq.v_keywords[i]);
		v_keywords.push_back(ptr);
	}

	return *this;
}