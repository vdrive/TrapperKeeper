// ConnectionModuleThreadData.h
#pragma once
#include "QueryHitResult.h"

class ConnectionModuleStatusData;
class GnutellaHost;
class CompressedQRPTable;
class VendorCount;
class ConnectionManager;

class ConnectionModuleThreadData
{
public:
	ConnectionModuleThreadData();
	~ConnectionModuleThreadData(){};

	WSAEVENT m_reserved_events[4];
	ConnectionModuleStatusData *p_status_data;
	vector<GnutellaHost> *p_connect_hosts;

	CompressedQRPTable *p_compressed_qrp_table;
	ConnectionManager* p_connection_manager;

	vector<QueryHitResult> *vp_share_files;		//storing all supply info
};