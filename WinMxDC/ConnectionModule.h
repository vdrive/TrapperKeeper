// ConnectionModule.h
#pragma once
#include "SupernodeHost.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"
#include "ProjectKeywords.h"
#include "ProjectSupplyQuery.h"

class CCriticalSection;
class ConnectionManager;
class ConnectionModule
{
public:
	ConnectionModule();
	~ConnectionModule();
	void InitParent(ConnectionManager *manager,unsigned int mod);
	void ConnectToHosts(vector<SupernodeHost> &hosts);
	bool IsConnected(SupernodeHost& host);
	unsigned int GetModNumber();
	void ReConnectAll();

	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);
	void KeywordsUpdated();
	//void ProjectSupplyUpdated();

	// Message Window Functions
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void OnTimer(UINT nIDEvent);
	void StatusReady(WPARAM wparam,LPARAM lparam);
	void LogMsg(WPARAM wparam,LPARAM lparam);
	bool MostlyConnected();
	void GetConnectedHosts(vector<SupernodeHost>& hosts);
	void StopSearchingProject(const char* project_name);


private:
	ConnectionManager *p_manager;
	unsigned int m_mod;

	ConnectionModuleMessageWnd m_wnd;
	HWND m_hwnd;
	CCriticalSection *p_critical_section;
	ConnectionModuleThreadData* p_thread_data;

	vector<DWORD> v_connect_to_ips;
	CWinThread* m_thread;
	hash_set<SupernodeHost,SupernodeHostHash> hs_connected_host_ips;
};