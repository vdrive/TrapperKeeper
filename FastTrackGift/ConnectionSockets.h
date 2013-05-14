// ConnectionSockets.h
#pragma once

#include "FastTrackSocket.h"
#include "ConnectionModuleStatusData.h"
#include "SupernodeHost.h"
//#include "ProjectKeywords.h"
#include "ProjectSupplyQuery.h"
#include "ConnectionModuleStatusData.h"
#include "SupplyProject.h"
#include "ProjectKeywords.h"
#include "osrng.h"	// for CryptoPP

//#include "BSA.h"

class VendorCount;
class FileSharingManager;
class ConnectionSockets
{
public:
	// Public Member Functions
	ConnectionSockets();
	~ConnectionSockets();
	void Log(const char* log);

	DWORD ReturnNumberOfEvents();
	int ReturnNumberOfReservedEvents();
	int ReturnNumberOfSocketEvents();

	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);

	void ConnectToHosts(vector<SupernodeHost> &hosts);
	void ReportHosts(vector<SupernodeHost> &hosts);
	//void Search(const char* search);

	 void ReportStatus(ConnectionModuleStatusData& status_data);

	void UpdateKeywords(vector<ProjectKeywords> &keywords);
	//void UpdateSupply(vector<SupplyProject> &supply);

	vector<VendorCount> *ReturnVendorCounts();
	void ReConnectAll();
	void GetRandomUserName(CString& username);
	void ReadInUserNames();
	void ChangeUserName(char * username);

//	void CreateSpoofShares();


	// Public Data Members
	enum constants
	{
		NumberOfEvents=WSA_MAXIMUM_WAIT_EVENTS,
		NumberOfReservedEvents=4,
		NumberOfSocketEvents=NumberOfEvents-NumberOfReservedEvents
	};

	WSAEVENT m_events[NumberOfEvents];
	FastTrackSocket m_sockets[NumberOfSocketEvents];

	vector<ProjectKeywords> v_keywords;
//	vector<SupplyProject> v_supply;
	//vector<ProjectSupplyQuery> v_project_supply_queries;
	hash_set<ProjectSupplyQuery,ProjectSupplyQueryHash>hs_project_supply_queries;
	vector<CString> v_spoof_filenames;	

	/*
	void SpoofIP(unsigned int ip);
	unsigned int SpoofIP();
	vector<unsigned int> v_spoof_ips;	// for the sockets to get ips from when needing a reasonable random spoof ip
	*/
	ConnectionModuleStatusData m_status_data;
	HWND m_dlg_hwnd;

	vector<CString> v_usernames;
	//vector<PacketBuffer*> v_shared_files_buffers;
	FileSharingManager* p_file_sharing_manager;
	
private:
	CryptoPP::AutoSeededRandomPool m_rng;
/*	char m_my_ip[16];
	unsigned short GetHashChecksum (unsigned char *hash);
	char* GetRandomFilename(const char* artist, const char* album, const char* title, int track);
	void GetMyIP();
*/
	UINT m_one_hour_timer;

};