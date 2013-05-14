// ConnectionSockets.h
#pragma once

#include "FastTrackSocket.h"
#include "ConnectionModuleStatusData.h"
#include "SupernodeHost.h"
//#include "ProjectKeywords.h"
#include "ConnectionModuleStatusData.h"
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

	void ConnectToHosts(vector<SupernodeHost> &hosts);
	void ReportHosts(vector<SupernodeHost> &hosts);
	//void Search(const char* search);

	 void ReportStatus(ConnectionModuleStatusData& status_data);


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

	vector<CString> v_spoof_filenames;	

	ConnectionModuleStatusData m_status_data;
	HWND m_dlg_hwnd;

	vector<CString> v_usernames;
	FileSharingManager* p_file_sharing_manager;
	
private:
	CryptoPP::AutoSeededRandomPool m_rng;
	UINT m_one_hour_timer;

};