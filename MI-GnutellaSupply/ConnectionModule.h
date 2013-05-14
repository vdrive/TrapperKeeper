// ConnectionModule.h
#pragma once

#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"
#include "GnutellaHost.h"
#include "ProjectKeywords.h"
#include "ProjectSupplyQuery.h"

class ConnectionManager;
class CCriticalSection;

class ConnectionModule
{
public:
	ConnectionModule();
	~ConnectionModule();
	void InitParent(ConnectionManager *manager,unsigned int mod);

	void ConnectToHosts(vector<GnutellaHost> &hosts);

	bool IsConnected(unsigned int ip);

	unsigned int GetModNumber();

	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);

	void KeywordsUpdated();
	//void ProjectSupplyUpdated(char *project);

	void ReportVendorCounts();

	// Message Window Functions
	void InitThreadData(WPARAM wparam,LPARAM lparam);
	void OnTimer(UINT nIDEvent);
	void StatusReady(WPARAM wparam,LPARAM lparam);
	void VendorCountsReady(WPARAM wparam,LPARAM lparam);

private:
	ConnectionManager *p_manager;
	unsigned int m_mod;

	ConnectionModuleMessageWnd m_wnd;
	HWND m_hwnd;
	CCriticalSection *p_critical_section;
	ConnectionModuleThreadData m_thread_data;

	vector<unsigned int> v_connected_host_ips;
	vector<unsigned int> v_connect_to_ips;
	CWinThread* m_thread;

};