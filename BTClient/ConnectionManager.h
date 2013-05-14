#pragma once
#include "manager.h"
#include "ConnectionModuleStatusData.h"
#include "ConnectionModule.h"
#include <vector>
#include "..\BTScraperDll\TorrentFile.h"
#include "ListeningSocket.h"
#include "ClientData.h"
#include "Config.h"
#include "ModuleData.h"
using namespace std;

//class VendorCount;
//class FileSharingManager;
class BTClientDll;
class ConnectionManager : public Manager
{
public:
	ConnectionManager(void);
	~ConnectionManager(void);
	void InitParent(BTClientDll * parent);
//	void SetFileSharingManager(FileSharingManager *file_sharing_manager);
	void AddModule(TorrentFile * tf, int listeningPort);
	void AddListeningModule(TorrentFile * tf, int listeningPort);
	void ReadConfig();
	Config GetConfig();


	void ReportStatus(ConnectionModuleStatusData &status);
//	void ReportHosts(vector<SupernodeHost> &hosts);
	unsigned int ReturnModCount();
	void KillModules();
	
	void KillModule(int modnum);

	void LimitModuleCount(int count);
//	void OnHeartbeat();	// 1 minute timer to write out the host cache to file
	void TimerHasFired();	// 1 sec timer
	
	void StartNewTorrent(TorrentFile * tf);
	void SetRefreshFlag();
//	void VendorCountsReady(vector<VendorCount> *vendor_counts);
	void LogMsg(char* msg);
	void ClientDataReported(ClientData * cdata);
	void ModuleDataReported(ModuleData * cdata);

	// Temp log message for debugging
	void LogMess(char * msg);
//	void ReConnectAll();

	void AddNewClient(SOCKET hSocket);

//	FileSharingManager *p_file_sharing_manager;
	bool m_vendor_counts_enabled;

	void IncomingConnection(vector<SOCKET> sockets, int socknum);

	

private:
	// Private Data Members
	vector<ConnectionModule *> v_mods;
	BTClientDll * p_parent;

	bool m_refresh_modules;

	int m_num_mods;
	int m_next_port;

	vector <TorrentFile> v_torrents;

	vector<ConnectionModule *> v_listening_mods;
	vector<ListeningSocket* > v_listening_sockets;

	Config m_config;
	//hash_set<SupernodeHost,SupernodeHostHash> hs_host_cache; //storing the main supernode list, hashed by ip
//	unsigned int m_vendor_counts_timer_counter;
//	vector<VendorCount> v_vendor_counts;
//	unsigned short m_search_id;
//	bool m_connect_to_supernode;

	// Private Member Functions
//	void WriteOutHostCache();
//	void ReadInHostCache();

	//CryptoPP::AutoSeededRandomPool m_rng;
};
