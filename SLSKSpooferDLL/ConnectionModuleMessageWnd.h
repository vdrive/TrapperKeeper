// ConnectionModuleMessageWnd.h
#pragma once

#define WM_INIT_THREAD_DATA		WM_USER+9
#define WM_STATUS_READY			WM_USER+10
//#define WM_VENDOR_COUNTS_READY	WM_USER+3
#define WM_LOG_MSG				WM_USER+4
#define WM_SERVER_STATUS_MSG     WM_USER+5
#define WM_PARENT_STATUS_MSG	WM_USER+6
#define WM_PEER_STATUS_MSG		WM_USER+7
#define WM_SEND_TASK			WM_USER+8
#define WM_USER_NAME			WM_USER+3
#define WM_ADD_PARENT			WM_USER+11
#define WM_CONNECTED_SOCKETS	WM_USER+12
#define WM_CONNECTING_SOCKETS	WM_USER+13
#define WM_DROP_CACHED_PARENT	WM_USER+14
#define WM_NUM_PARENT_CONNECTING WM_USER+15
#define WM_NUM_PEER_CONNECTING  WM_USER+16
#define WM_WRITE_TO_LOG			WM_USER+17


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
	//afx_msg LRESULT VendorCountsReady(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT LogMsg(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ServerStatusMsg(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT ParentStatusMsg(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT PeerStatusMsg(WPARAM wparam,LPARAM lparam);
	afx_msg LRESULT SendTask(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT SetUserName(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT AddParent(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT ConnectingSockets(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT ConnectedSockets(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT DropCachedParent(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT setParentConnecting(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT setPeerConnecting(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT WriteToLog(WPARAM wparam, LPARAM lparam);


	DECLARE_MESSAGE_MAP()
};