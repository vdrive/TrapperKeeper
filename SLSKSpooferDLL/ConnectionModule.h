// ConnectionModule.h
#pragma once
#include "SupernodeHost.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"
#include "ProjectKeywordsVector.h"
//#include "TrackInformation.h"
//#include "ProjectKeywords.h"
//#include "ProjectSupplyQuery.h"

/*struct parameter
{
public:

};*/

class CCriticalSection;
class ConnectionManager;
class ConnectionModule
{
public:
	ConnectionModule();
	ConnectionModule(int state);
	ConnectionModule(int state, ProjectKeywordsVector *projects);
	//ConnectionModule(int state, ProjectKeywordsVector *projects, vector<TrackInformation *> *tracks);
	~ConnectionModule();
	void InitParent(ConnectionManager *manager,unsigned int mod);
	bool ConnectToHosts(vector<SupernodeHost> &hosts);
	bool ConnectToHosts(CString un, vector<SupernodeHost> &hosts);
	bool ConnectToHosts(CString un, SLSKtask *t);
	bool GetTask(SLSKtask *t);
	int NumConnected();
	bool IsConnected(unsigned int ip);
	unsigned int GetModNumber();
	void ReConnectAll();

	//void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);
	//void KeywordsUpdated();
	//void ProjectSupplyUpdated();

	// Message Window Functions
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void OnTimer(UINT nIDEvent);
	void StatusReady(WPARAM wparam,LPARAM lparam);
	//void VendorCountsReady(WPARAM wparam,LPARAM lparam);
	void LogMsg(WPARAM wparam,LPARAM lparam);
	void ParentStatusMsg(WPARAM wparam, LPARAM lparam);
	void ServerStatusMsg(WPARAM wparam, LPARAM lparam);
	void PeerStatusMsg(WPARAM wparam, LPARAM lparam);
	void SendTask(WPARAM wparam, LPARAM lparam);
	void SetUserName(WPARAM wparam, LPARAM lparam);
	void AddParent(WPARAM wparam, LPARAM lparam);
	void ChangeConnectedSockets(WPARAM wparam, LPARAM lparam);
	void ChangeConnectingSockets(WPARAM wparam, LPARAM lparam);
	void setParentConnecting(WPARAM wparam, LPARAM lparam);
	void setPeerConnecting(WPARAM wparam, LPARAM lparam);
	void WriteToLog(WPARAM wparam, LPARAM lparam);
	void DropCachedParent(SupernodeHost *nh);
	bool SetProjects(ProjectKeywordsVector *projects);
	int m_parent_connecting, m_parent_connected, m_server_connected, m_peer_connecting, m_peer_connected;
	//bool SetTracks(vector<TrackInformation *> *tracks);
	//void ReportVendorCounts();
//	void Search(const char* search);
	bool MostlyConnected();

	int m_connected_sockets;
	int m_connecting_sockets;

private:
//	SLSKaccountInfo account_info;
	ConnectionManager *p_manager;
	unsigned int m_mod;

	ConnectionModuleMessageWnd m_wnd;
	HWND m_hwnd;
	int m_state;
	CCriticalSection *p_critical_section;
	ConnectionModuleThreadData* p_thread_data;

	vector<unsigned int> v_connected_host_ips;
	vector<unsigned int> v_connect_to_ips;
	CWinThread* m_thread;
	vector<SLSKtask *> tasks;



//	vector<SupernodeHost> v_initial_accepted_connections;

};