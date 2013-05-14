// ConnectionModule.h
#pragma once
//#include "SupernodeHost.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"
#include "ConnectionModuleStatusData.h"
#include "..\BTScraperDll\TorrentFile.h"
#include "config.h"



class CCriticalSection;
class ConnectionManager;
class ConnectionModule
{
public:
	ConnectionModule();
	ConnectionModule(int modnum);
	~ConnectionModule();
	void InitParent(ConnectionManager *manager,unsigned int mod);
	//void ConnectToHosts(vector<SupernodeHost> &hosts);
	bool IsConnected(unsigned int ip);
	unsigned int GetModNumber();
	void ReConnectAll();



	// Message Window Functions
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void OnTimer(UINT nIDEvent);
	void StatusReady(WPARAM wparam,LPARAM lparam);
	void VendorCountsReady(WPARAM wparam,LPARAM lparam);
	void LogMsg(WPARAM wparam,LPARAM lparam);
	void ReportVendorCounts();
	bool MostlyConnected();
	void DataReported(WPARAM wparam,LPARAM lparam);
	void ModuleDataReported(WPARAM wparam,LPARAM lparam);

	void AddNewClient(SOCKET hSocket);
	bool HasIdleSocket(void);


	// Temp for Debugging
	void LogMess(char * msg);

	char * CreatePeerID();
	void test(TorrentFile * tf);
	unsigned char * GetHandshakeMessage();
	void InitTorrent(TorrentFile * tf, int listening_port);

	void SetAsListener();

	int GetModnum();
	int GetListeningPort();

	TorrentFile GetTorrentFile();


	void KillYourself();
	void ConnectToNewClients(TorrentFile *torrent);


private:
	ConnectionManager *p_manager;
	unsigned int m_mod;

	ConnectionModuleMessageWnd m_wnd;
	HWND m_hwnd;
	CCriticalSection *p_critical_section;
	ConnectionModuleThreadData* p_thread_data;

	vector<unsigned int> v_connected_host_ips;
	vector<unsigned int> v_connect_to_ips;
	CWinThread* m_thread;

	ConnectionModuleStatusData m_status_data;

	vector<SOCKET> v_initial_accepted_connections;	// accepted socket handles that were accepted before the thread had started up


	TorrentFile m_torrent;
	PeerList * m_peers;
	bool * p_bitfield;	
	char m_peer_id[20+1];

	int m_modnum;
	int m_listening_port;

	bool m_is_listener;

	Config m_config;

};