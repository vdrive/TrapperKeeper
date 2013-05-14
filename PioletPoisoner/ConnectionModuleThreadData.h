// ConnectionModuleThreadData.h

#ifndef CONNECTION_MODULE_THREAD_DATA_H
#define CONNECTION_MODULE_THREAD_DATA_H

#include "winsock2.h"	// for WSAEVENT

class ConnectionModuleStatusData;
//class ProjectKeywords;
//class ProjectSupplyQuery;
//class SupplyProject;
//class VendorCount;

class ConnectionModuleThreadData
{
public:
	ConnectionModuleThreadData();

	WSAEVENT m_reserved_events[4];

//	ConnectionModuleStatusData *p_status_data;
//	vector<ProjectKeywords> *p_keywords;
//	vector<SupplyProject> *p_supply;
//	vector<ProjectSupplyQuery> *p_project_supply_queries;
//	vector<VendorCount> *p_vendor_counts;

//	UINT* p_max_host;
//	UINT* p_max_host_cache;

/*
	vector<GnutellaHost> *p_connect_hosts;
	CompressedQRPTable *p_compressed_qrp_table;
*/
	HWND m_hwnd;

};

#endif // CONNECTION_MODULE_THREAD_DATA_H