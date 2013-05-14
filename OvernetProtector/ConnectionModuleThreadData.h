// ConnectionModuleThreadData.h

#ifndef CONNECTION_MODULE_THREAD_DATA_H
#define CONNECTION_MODULE_THREAD_DATA_H

#include "winsock2.h"	// for WSAEVENT

class ProjectKeywords;
class ProjectSupplyQuery;
class ConnectionModuleStatusData;
class ConnectionModuleThreadData
{
public:
	ConnectionModuleThreadData();

	WSAEVENT m_reserved_events[4];

	vector<ProjectKeywords> *p_keywords;
	vector<ProjectSupplyQuery> *p_project_supply_queries;
	ConnectionModuleStatusData *p_status_data;

};

#endif // CONNECTION_MODULE_THREAD_DATA_H