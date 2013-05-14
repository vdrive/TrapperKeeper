// ConnectionSockets.h
#pragma once

//#include "eD2kClient.h"
#include "ConnectionModuleStatusData.h"
#include "ClientSocket.h"
#include "..\BTScraperDll\TorrentFile.h"
#include "Config.h"
#include "ModuleData.h"
//#include "SupernodeHost.h"
//#include "ProjectKeywords.h"
//#include "ConnectionModuleStatusData.h"
//#include "osrng.h"	// for CryptoPP

//#include "BSA.h"

//class VendorCount;
//class FileSharingManager;
class ConnectionSockets
{
public:
	// Public Member Functions
	ConnectionSockets();
	~ConnectionSockets();
//	void Log(const char* log);

	DWORD ReturnNumberOfEvents();
	int ReturnNumberOfReservedEvents();
	int ReturnNumberOfSocketEvents();

	void AddNewClient(SOCKET hSocket);
	char * GetTrackerMessage(TorrentFile * tf,const char *url,char * peer_id, int portint);
	void RegisterWithTracker(TorrentFile * tf, int port);
	void MakeNewConnection(char * ip, int port, bool is_seed);
	char * CreatePeerID();
	void InitTorrent(TorrentFile torrent);
	unsigned char * GetHandshakeMessage();
	int CheckNumberOfConnections();

	int UnZip(unsigned short ** dest, unsigned short * source, int len); 
	int SendTrackerInfo(char * url,unsigned short **body, string referer);
//	void ConnectToHosts(vector<SupernodeHost> &hosts);
//	void ReportHosts(vector<SupernodeHost> &hosts);
	//void Search(const char* search);

	void ReportStatus(ConnectionModuleStatusData& status_data);
	void KeepConnectionsAlive();
	void ReportDataReceived(int data_requested, int next_request,int piece,char * ip, char * name, bool is_seed);
	void ReportDataSent(int data_sent,int piece,char * ip, char * name);

	
	void ReportClientId(char * id, char * ip);
	void ReportOnConnect(char * ip);
	void ReportToDialog(char * msg);	



	 void LogMessage(char * msg);

	 


//	vector<VendorCount> *ReturnVendorCounts();
//	void ReConnectAll();
//	void GetRandomUserName(CString& username);
//	void ReadInUserNames();
//	void ChangeUserName(char * username);

//	void CreateSpoofShares();


	// Public Data Members
	enum constants
	{
		NumberOfEvents=WSA_MAXIMUM_WAIT_EVENTS,
		NumberOfReservedEvents=4,
		NumberOfSocketEvents=NumberOfEvents-NumberOfReservedEvents
	};

	WSAEVENT m_events[NumberOfEvents];
	CClientSocket m_sockets[NumberOfSocketEvents];

//	vector<CString> v_spoof_filenames;	

	ConnectionModuleStatusData m_status_data;
	HWND m_dlg_hwnd;

//	vector<CString> v_usernames;
//	FileSharingManager* p_file_sharing_manager;
	
private:
	char m_peer_id[20+1];
	TorrentFile m_torrent;

	Config m_config;
	ModuleData m_mod_data;
	//CryptoPP::AutoSeededRandomPool m_rng;
//	UINT m_one_hour_timer;

};