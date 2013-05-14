// ConnectionManager.cpp

#include "stdafx.h"
#include "ConnectionManager.h"
#include "OvernetProtectorDll.h"
CCriticalSection ConnectionManager::m_critSect;

//
//
//
ConnectionManager::ConnectionManager()
{
	m_mod.InitParent(this);
	m_num_peers=m_seconds=m_searching_project/*=m_searching_track*/=0;
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
void ConnectionManager::InitParent(OvernetProtectorDll *parent)
{
	p_parent=parent;
}

//
//
//
void ConnectionManager::TimerHasFired()
{
	m_seconds++;
	m_mod.TimerHasFired();
	if(m_projects.v_projects.size() && (m_seconds%10 == 0) && m_num_peers >= 100000) //search each project every 10 secs
	{
		if(m_searching_project >= m_projects.v_projects.size())
		{
			m_searching_project=0;
			p_parent->Log("** Reset searching for projects from the beginning **");
		}
		PerformProjectSupplyQuery();
		m_searching_project++;
	}
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
void ConnectionManager::PerformProjectSupplyQuery()
{
	//UINT i,j;

	// Init the project supply query object with a GUID
	ProjectSupplyQuery psq;
	// Find the project in the keywords vector and give them to the query object to extract
	if(m_projects.v_projects.size() > 0)
	{
		/*
		if(m_searching_track > m_projects.v_projects[m_searching_project].m_supply_keywords.v_keywords.size()-1)
		{
			m_searching_project++;
			m_searching_track=0;
		}
		*/
		psq.ExtractProjectKeywordData(&m_projects.v_projects[m_searching_project]/*,m_searching_track*/);
		char* key=NULL;
		if(psq.v_search_words.size())
		{
			key=new char[psq.v_search_words[0].GetLength()+1];
			strcpy(key,psq.v_search_words[0]);
			if(key!=NULL)
			{
				m_df.CreateHashFromInput(NULL,NULL,strlen(key),psq.m_keyword_hash,(byte*)key);
				delete [] key;
				m_mod.PerformProjectSupplyQuery(psq);
			}
		}
		else
		{
			for(UINT i=0;i<m_projects.v_projects[m_searching_project].m_supply_keywords.v_keywords.size();i++)
			{
				psq.ExtractProjectKeywordTrackData(&m_projects.v_projects[m_searching_project],i);
				if(psq.v_search_words.size())
				{
					key=new char[psq.v_search_words[0].GetLength()+1];
					strcpy(key,psq.v_search_words[0]);
					if(key!=NULL)
					{
						m_df.CreateHashFromInput(NULL,NULL,strlen(key),psq.m_keyword_hash,(byte*)key);
						delete [] key;
						m_mod.PerformProjectSupplyQuery(psq);
					}
				}
			}
		}
		//m_searching_track++;
	}	
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