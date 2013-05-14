// ConnectionModuleMessageWnd.h
#pragma once

#define WM_INIT_THREAD_DATA		WM_USER+1
#define WM_STATUS_READY			WM_USER+2
#define WM_VENDOR_COUNTS_READY	WM_USER+3

class ConnectionModule;

class ConnectionModuleMessageWnd : public CWnd
{
public:
	ConnectionModuleMessageWnd();
	~ConnectionModuleMessageWnd(){};
	void InitParent(ConnectionModule *mod);

private:
	ConnectionModule *p_mod;

	afx_msg LRESULT InitThreadData(WPARAM wparam,LPARAM lparam);
	void OnTimer(UINT nIDEvent);
	afx_msg LRESULT StatusReady(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT VendorCountsReady(WPARAM wparam,LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};