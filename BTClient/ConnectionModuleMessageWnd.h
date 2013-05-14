// ConnectionModuleMessageWnd.h
#pragma once

#define WM_INIT_THREAD_DATA		WM_USER+9
#define WM_STATUS_READY			WM_USER+10
#define WM_VENDOR_COUNTS_READY	WM_USER+3
#define WM_LOG_MSG				WM_USER+4
#define WM_KILL_MODULE			WM_USER+11
#define WM_REPORT_DATA			WM_USER+12
#define WM_REPORT_MOD_DATA		WM_USER+13

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
	afx_msg LRESULT LogMsg(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT	KillMsg(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT	ReportData(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT	ReportModData(WPARAM wparam,LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};