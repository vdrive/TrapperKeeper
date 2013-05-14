// ConnectionSockets.h
#pragma once

#include "GnutellaSocket.h"
#include "ConnectionModuleStatusData.h"
#include "GnutellaHost.h"
#include "ProjectKeywords.h"
#include "CompressedQRPTable.h"
#include "ProjectSupplyQuery.h"
#include "ConnectionModuleStatusData.h"
//#include "SupplyProject.h"

//#include "BSA.h"

class VendorCount;

class ConnectionSockets
{
public:
	// Public Member Functions
	ConnectionSockets();
	~ConnectionSockets();

	DWORD ReturnNumberOfEvents();
	int ReturnNumberOfReservedEvents();
	int ReturnNumberOfSocketEvents();

	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);

	void ConnectToHosts(vector<GnutellaHost> &hosts);
	void ReportHosts(vector<GnutellaHost> &hosts);

	void ReportStatus(ConnectionModuleStatusData& status);

	void UpdateKeywords(vector<ProjectKeywords> &keywords);
	void UpdateCompressedQRPTable(CompressedQRPTable &table);
	//void UpdateSupply(vector<SupplyProject> &supply);

//	void SpoofIP(unsigned int ip);
//	unsigned int SpoofIP();

	vector<VendorCount> *ReturnVendorCounts();

	// Public Data Members
	enum constants
	{
		NumberOfEvents=WSA_MAXIMUM_WAIT_EVENTS,
		NumberOfReservedEvents=4,
		NumberOfSocketEvents=NumberOfEvents-NumberOfReservedEvents
	};

	WSAEVENT m_events[NumberOfEvents];
	GnutellaSocket m_sockets[NumberOfSocketEvents];

	vector<ProjectKeywords> v_keywords;
	//vector<SupplyProject> v_supply;
	CompressedQRPTable m_compressed_qrp_table;
	vector<ProjectSupplyQuery> v_project_supply_queries;

	ConnectionModuleStatusData m_status_data;

	//vector<unsigned int> v_spoof_ips;	// for the sockets to get ips from when needing a reasonable random spoof ip

//	BSA m_bsa;
};