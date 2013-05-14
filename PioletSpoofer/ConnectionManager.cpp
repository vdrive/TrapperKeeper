// ConnectionManager.cpp

#include "stdafx.h"
#include "ConnectionManager.h"
#include "PioletSpooferDll.h"
#include "KeywordManager.h"
#include "SupplyManager.h"
#include "ProjectSupplyQuery.h"

//
//
//
ConnectionManager::ConnectionManager()
{
	p_keyword_manager=NULL;
	p_supply_manager=NULL;
	p_parent=NULL;
	m_mod.InitParent(this);
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
void ConnectionManager::InitParent(PioletSpooferDll *parent)
{
	p_parent=parent;
}

//
//
//
void ConnectionManager::InitKeywordManager(KeywordManager* parent)
{
	p_keyword_manager=parent;
}

//
//
//
void ConnectionManager::InitSupplyManager(SupplyManager* parent)
{
	p_supply_manager=parent;
}

//
//
//
void ConnectionManager::TimerHasFired()
{
	m_mod.TimerHasFired();
}

//
//
//
void ConnectionManager::ReportStatus(char *status,unsigned int *counts)
{
	if(status!=NULL)
	{
		p_parent->m_dlg.Log(status);
	}
	if(counts!=NULL)
	{
		p_parent->m_dlg.IncrementCounters(counts[0],counts[1],counts[2],counts[3],counts[4],counts[5]);
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
	return p_keyword_manager->v_project_keywords;
}

//
//
//
vector<SupplyProject>* ConnectionManager::ReturnSupplyProjectsPointer()
{
	return &p_supply_manager->v_supply_projects;
}

//
//
//
void ConnectionManager::SpoofDataReady(ConnectionModuleStatusData& status)
{
	p_parent->SpoofDataReady(status);
}

//
//
//
void ConnectionManager::ProjectSupplyUpdated(char* project_name)
{
#ifndef NO_PROCESSING
	p_parent->SyncherRefresh();
#endif
	m_mod.ProjectSupplyUpdated(project_name);
}

//
//
//
void ConnectionManager::PerformProjectSupplyQuery(char *project)
{
	UINT i;

	// Init the project supply query object with a GUID
	ProjectSupplyQuery psq;
/*	
	if(guid==NULL)
	{
		// Create the GUID and make it look like a new gnutella client
		CoCreateGuid(&psq.m_guid);
		unsigned char *ptr=(unsigned char *)&psq.m_guid;
		ptr[8]=0xFF;
		ptr[15]=0x00;;
	}
	else
	{
		psq.m_guid=*guid;
	}

	//store this guid into database
	//p_db_manager->InsertGUID(psq.m_guid, project,CTime::GetCurrentTime());
	CString ts = CTime::GetCurrentTime().Format("%Y%m%d%H%M%S");
	CString cs_project = project;
	m_db_interface.InsertGUID(GNUTELLA_GUID_INDEX_TALBE, psq.m_guid, cs_project, ts);

	p_parent->m_log_window_manager.Log("Connection Manager : ",0x0000C0C0);	// dirty yellow
	p_parent->m_log_window_manager.Log("Starting Supply Query for project: ");
	p_parent->m_log_window_manager.Log(project,0x00FF0000,true);	// bold blue
	p_parent->m_log_window_manager.Log("\n");
*/
	char msg[512];
	sprintf(msg, "Connection Manager: Starting Supply Query for project: %s\n", project);
	p_parent->m_dlg.Log(msg);

//=>_<= 	psq.m_is_searcher_query=is_searcher_query;

	// Find the project in the keywords vector and give them to the query object to extract
	for(i=0;i<p_keyword_manager->v_project_keywords.size();i++)
	{
		if(strcmp(p_keyword_manager->v_project_keywords[i].m_project_name.c_str(),project)==0)
		{
			psq.ExtractProjectKeywordData(&p_keyword_manager->v_project_keywords[i]);
			psq.m_project_status_index=i;
			break;
		}
	}
	
	// Tell all of the mods to take supply for this project, unless it is a searcher query, where we only tell the first mod
	m_mod.PerformProjectSupplyQuery(psq);
	/*
	if(is_searcher_query)
	{
		if(v_mods.size()>0)
		{
			v_mods[0]->PerformProjectSupplyQuery(psq);
		}
	}
	else
	{
		for(i=0;i<(int)v_mods.size();i++)
		{
			v_mods[i]->PerformProjectSupplyQuery(psq);
		}
	}

	return psq.m_guid;
	*/
}

//
//
//
void ConnectionManager::UpdateHostLimits(UINT max_host, UINT max_host_cache)
{
	m_mod.UpdateHostLimits(max_host,max_host_cache);
}

//
//
//
void ConnectionManager::ReportVendorCounts(vector<VendorCount>* vendor_counts)
{
	p_parent->ReportVendorCounts(vendor_counts);
}

//
//
//
void ConnectionManager::AddDC(CString dc)
{
	m_mod.AddDC(dc);
}

//
//
//
void ConnectionManager::SendPoisonEntry(char* poisoner, PoisonEntries& pe)
{
	p_parent->SendPoisonEntry(poisoner, pe);
}

//
//
//
vector<PoisonerStatus>* ConnectionManager::GetPoisoners()
{
	return p_parent->GetPoisoners();
}

//
//
//
int ConnectionManager::GetNumPoisoners()
{
	return p_parent->GetNumPoisoners();
}
//
//
//
/*
void ConnectionManager::LockSupplyProject()
{
	p_supply_manager->LockSupplyProject();
}

//
//
//
void ConnectionManager::ReleaseSupplyProject()
{
	p_supply_manager->ReleaseSupplyProject();
}
*/

void ConnectionManager::Log(char *buf, COLORREF color, bool bold, bool italic)
{
	/*
	char *ptr=new char[strlen(buf)+1];
	strcpy(ptr,buf);
	::PostMessage(m_hwnd,WM_STATUS_READY,(WPARAM)ptr,(LPARAM)0);
	*/
	p_parent->m_dlg.Log(buf, color, bold, italic);
}