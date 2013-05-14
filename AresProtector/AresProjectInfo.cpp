#include "StdAfx.h"
#include ".\aresprojectinfo.h"

AresProjectInfo::AresProjectInfo(const char* name, const char* owner, const char* artist,int id)
{
	m_id=id;
	m_name=name;
	m_owner=owner;
	m_artist=artist;
}

AresProjectInfo::~AresProjectInfo(void)
{
}
