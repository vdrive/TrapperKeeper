// ConnectionModule.h
#pragma once
#include "SupernodeHost.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModuleThreadData.h"

class CCriticalSection;
class ConnectionManager;
class ConnectionModule
{
public:
	ConnectionModule();
	~ConnectionModule();
	void InitParent(ConnectionManager *manager,unsigned int mod);
	void ConnectToHosts(vector<SupernodeHost> &hosts);
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

};