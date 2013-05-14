// ConnectionSockets.h
#pragma once

#include "GnutellaSocket.h"
#include "ConnectionModuleStatusData.h"
#include "GnutellaHost.h"
#include "ProjectKeywords.h"
#include "CompressedQRPTable.h"
#include "ConnectionModuleStatusData.h"
#include "SupplyProject.h"
#include "osrng.h"	// for CryptoPP

//#include "BSA.h"

class VendorCount;
class ConnectionManager;
class ConnectionSockets
{
public:
	// Public Member Functions
	ConnectionSockets();
	~ConnectionSockets();

	DWORD ReturnNumberOfEvents();
	int ReturnNumberOfReservedEvents();
	int ReturnNumberOfSocketEvents();

	void ConnectToHosts(vector<GnutellaHost> &hosts);
	void ReportHosts(vector<GnutellaHost> &hosts);

	//ConnectionModuleStatusData ReportStatus();
	void ReportStatus(ConnectionModuleStatusData& status);

	void UpdateKeywords(vector<ProjectKeywords> &keywords);
	void UpdateCompressedQRPTable(CompressedQRPTable &table);

	void SpoofIP(unsigned int ip);
	unsigned int SpoofIP();
	bool IsProjectSupplyBeingUpdated();

	vector<VendorCount> *ReturnVendorCounts();
	int ReturnCompressionOnCounts();
	void SpoofEDonkeyIP(GnutellaHost* host);



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
	vector<SupplyProject>* p_supply;
	CompressedQRPTable m_compressed_qrp_table;

	ConnectionModuleStatusData m_status_data;

	vector<unsigned int> v_spoof_ips;	// for the sockets to get ips from when needing a reasonable random spoof ip
	ConnectionManager* p_connection_manager;
	CryptoPP::AutoSeededRandomPool m_rng;
	vector<GnutellaHost> v_edonkey_spoof_ips;

//	BSA m_bsa;
};