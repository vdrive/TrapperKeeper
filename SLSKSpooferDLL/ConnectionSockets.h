// ConnectionSockets.h
#pragma once

#include "SLSKSocket.h"
#include "ConnectionModuleStatusData.h"
#include "SupernodeHost.h"
//#include "ProjectKeywords.h"
//#include "ProjectSupplyQuery.h"
#include "ConnectionModuleStatusData.h"
#include "ProjectKeywordsVector.h"
//#include "TrackInformation.h"
#include "./zlib/zlib.h"
//#include "SupplyProject.h"
//#include "ProjectKeywords.h"
//#include "osrng.h"	// for CryptoPP

//#include "BSA.h"

//class VendorCount;
//class FileSharingManager;
struct demand
{
public:
	CString m_artist;
	int m_counter;
};

struct spoof
{
public:
	CString m_artist;
	vector<buffer *> m_spoof;
};

class ConnectionSockets
{

private: //TY
	bool b_inited;

public:
	// Public Member Functions
	ConnectionSockets();
	ConnectionSockets(int socketstate, ProjectKeywordsVector *pkv);
	~ConnectionSockets();
	void Log(const char* log);
	void ParentStatus(const char* status);
	void WriteToLog(const char* log);
	void PeerStatus(const char* status);
	void ServerStatus(const char* status);
	void setParentConnecting(const char* status);
	void setPeerConnecting(const char* status);
	void SetUserName(CString un);
	void AddParent(SupernodeHost newhost);
	void GetPeerIP(SLSKtask *task);
	void SendTask(SLSKtask *task);
	void ChangeConnectedSockets(void);
	void ChangeConnectingSockets(void);
	void DropCachedParent(SupernodeHost *nh);
	vector<project_info> projects;
	vector<w_keyword> weighted_keywords;
	vector<spoof_info> spoofing_info;
	ProjectKeywordsVector *m_projects;
	CTime m_attempted_server_connection;
	CTime m_last_status_sent;
	//vector<TrackInformation*> *m_track_info;
	void fillName(void);
	int random(int lowest, int highest);
	bool m_fill_projects;
	bool m_fill_spoofs;
	int ListeningPort;
	void SetProjects(ProjectKeywordsVector *projects);
//	void SetTracks(vector<TrackInformation *> *tracks);
	void CreateSpoofs(void);
	vector<spoof> m_spoofs;
	vector<demand> daily_demand;
	void FillDailyDemandVector(void);
	bool m_spoofs_created;
    

	DWORD ReturnNumberOfEvents();
	int ReturnNumberOfReservedEvents();
	int ReturnNumberOfSocketEvents();

	//void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);

	void ConnectToHosts(vector<SupernodeHost> &hosts);
	void ConnectToHosts(CString un, vector<SupernodeHost> &hosts);
	void ConnectToHosts(CString un, SLSKtask *task);
	void ReportHosts(vector<SupernodeHost> &hosts);
	//void InitAccountInfo(SLSKaccountInfo &nai);
	//void Search(const char* search);

	 void ReportStatus(ConnectionModuleStatusData& status_data);

	//void UpdateKeywords(vector<ProjectKeywords> &keywords);
	//void UpdateSupply(vector<SupplyProject> &supply);

//	vector<VendorCount> *ReturnVendorCounts();
	void ReConnectAll();
	//void GetRandomUserName(CString& username);
	//void ReadInUserNames();
	//void ChangeUserName(char * username);

//	void CreateSpoofShares();


	// Public Data Members
	enum constants
	{
		NumberOfEvents=WSA_MAXIMUM_WAIT_EVENTS,
		NumberOfReservedEvents=4,
		NumberOfSocketEvents=NumberOfEvents-NumberOfReservedEvents
	};

	WSAEVENT m_events[NumberOfEvents];
	SLSKSocket m_sockets[NumberOfSocketEvents];

	//vector<ProjectKeywords> v_keywords;
//	vector<SupplyProject> v_supply;
	//vector<ProjectSupplyQuery> v_project_supply_queries;
	//hash_set<ProjectSupplyQuery,ProjectSupplyQueryHash>hs_project_supply_queries;
	//vector<CString> v_spoof_filenames;	

	/*
	void SpoofIP(unsigned int ip);
	unsigned int SpoofIP();
	vector<unsigned int> v_spoof_ips;	// for the sockets to get ips from when needing a reasonable random spoof ip
	*/
	ConnectionModuleStatusData m_status_data;
	HWND m_dlg_hwnd;
	int state;


	CString GetFilePath(CString artist, CString album);
	CString getPassword(void);
	CString getUserName(void);
	CString getSpoofName(void);
	//void FillProjects(void);
	//void FillKeywords(void);
	void FillSpoofs(void);

	//vector<CString> v_usernames;
	//vector<PacketBuffer*> v_shared_files_buffers;
	//FileSharingManager* p_file_sharing_manager;
	
private:
//	SLSKaccountInfo account_info;
	//CryptoPP::AutoSeededRandomPool m_rng;
/*	char m_my_ip[16];
	unsigned short GetHashChecksum (unsigned char *hash);
	char* GetRandomFilename(const char* artist, const char* album, const char* title, int track);
	void GetMyIP();
*/
	UINT m_one_hour_timer;
	vector<CString> names;

	int m_connected_sockets;
	int m_connecting_sockets;

};