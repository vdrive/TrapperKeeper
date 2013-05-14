// ConnectionSockets.h
#pragma once

#include "GnutellaSocket.h"
#include "ConnectionModuleStatusData.h"
#include "GnutellaHost.h"
#include "CompressedQRPTable.h"
#include "ConnectionModuleStatusData.h"
#include "osrng.h"	// for CryptoPP
#include "QueryHitResult.h"

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
	void UpdateCompressedQRPTable(CompressedQRPTable &table);
	void UpdateShareList(vector<QueryHitResult>& share_files);

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

	CompressedQRPTable m_compressed_qrp_table;

	ConnectionModuleStatusData m_status_data;

	ConnectionManager* p_connection_manager;
	CryptoPP::AutoSeededRandomPool m_rng;
	vector<QueryHitResult> v_share_files;		//storing all supply info
};