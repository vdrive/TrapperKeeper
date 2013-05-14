// ConnectionModuleThreadData.h
#pragma once

class ConnectionModuleStatusData;
class GnutellaHost;
class ProjectKeywords;
class CompressedQRPTable;
class ProjectSupplyQuery;
class SupplyProject;
class VendorCount;

class ConnectionModuleThreadData
{
public:
	ConnectionModuleThreadData();
	~ConnectionModuleThreadData(){};

	WSAEVENT m_reserved_events[4];
	ConnectionModuleStatusData *p_status_data;
	vector<GnutellaHost> *p_connect_hosts;
	vector<ProjectSupplyQuery> *p_project_supply_queries;
	vector<ProjectKeywords> *p_keywords;
	vector<SupplyProject> *p_supply;

	CompressedQRPTable *p_compressed_qrp_table;
};