// ConnectionModuleThreadData.h
#pragma once

#include "SLSKtask.h"
//#include "TrackInformation.h"
#include "ProjectKeywordsVector.h"

class ConnectionModuleStatusData;
class SupernodeHost;
//class ProjectKeywords;
//class ProjectSupplyQuery;
//class SupplyProject;
//class VendorCount;
//class FileSharingManager;
class ConnectionModuleThreadData
{
public:
	ConnectionModuleThreadData();
	~ConnectionModuleThreadData(){};

	WSAEVENT m_reserved_events[5];
	ConnectionModuleStatusData *p_status_data;
	vector<SupernodeHost> *p_connect_hosts;
	CString	m_search_string;
	//vector<ProjectSupplyQuery> *p_project_supply_queries;
	//vector<ProjectKeywords> *p_keywords;
	//vector<SupplyProject> *p_supply;
	bool m_reconnect_to_hosts;
	bool m_disconnect_to_hosts;
	CString *m_user_name;
	vector<SLSKtask *> *tasks;
	ProjectKeywordsVector *p_projects;
	
	int m_state;
	//FileSharingManager* p_file_sharing_manager;
};