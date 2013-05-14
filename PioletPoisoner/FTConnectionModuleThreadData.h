// ConnectionModuleThreadData.h

#ifndef FT_CONNECTION_MODULE_THREAD_DATA_H
#define FT_CONNECTION_MODULE_THREAD_DATA_H

#include "winsock2.h"	// for WSAEVENT

//class FTConnectionModuleStatusData;
//class ProjectKeywords;
//class ProjectSupplyQuery;
//class SupplyProject;
//class VendorCount;

class FTConnectionModuleThreadData
{
public:
	FTConnectionModuleThreadData();

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
	string m_ip;
	int m_port;
	UINT m_file_length;
	string m_md5;
	int m_num_idle_sockets;
	//SOCKET m_socket;
	vector<SOCKET> *p_incoming_connection_socket_handles;

};

#endif // CONNECTION_MODULE_THREAD_DATA_H