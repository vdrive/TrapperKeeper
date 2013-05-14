// ConnectionManager.cpp

#include "stdafx.h"
#include "ConnectionManager.h"
#include "OvernetSpooferDll.h"
CCriticalSection ConnectionManager::m_critSect;

//
//
//
ConnectionManager::ConnectionManager()
{
	m_mod.InitParent(this);
	m_num_peers=m_seconds=0;
}

//
//
//
ConnectionManager::~ConnectionManager()
{
}

//
//
//
void ConnectionManager::TimerHasFired()
{
	m_seconds++;
	m_mod.TimerHasFired();
}

//
// Called by KeywordManager
//
void ConnectionManager::KeywordsUpdated()
{
	m_mod.KeywordsUpdated();
}

//
//
//
vector<ProjectKeywords> ConnectionManager::ReturnProjectKeywords()
{
	return m_projects.v_projects;
}

//
//
//
void ConnectionManager::SetProjectKeywords(ProjectKeywordsVector& projects)
{
	m_critSect.Lock();
	m_projects=projects;
	m_critSect.Unlock();
}

//
//
//
void ConnectionManager::StatusReady(ConnectionModuleStatusData& status)
{
	m_num_peers=status.m_num_peers;
	p_parent->StatusReady(status);
}

//
//
//
void ConnectionManager::Log(const char* log)
{
	p_parent->Log(log);
}

//
//
//
void ConnectionManager::SupplySynched()
{
	m_mod.SupplySynched();
}
