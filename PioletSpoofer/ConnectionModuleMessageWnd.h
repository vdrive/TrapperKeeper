// ConnectionModuleMessageWnd.h

#ifndef CONNECTION_MODULE_MESSAGE_WND_H
#define CONNECTION_MODULE_MESSAGE_WND_H

#define WM_INIT_THREAD_DATA		WM_USER+1
#define WM_STATUS_READY			WM_USER+2
#define WM_VENDOR_COUNTS_READY	WM_USER+3
#define WM_SPOOF_DATA_READY WM_USER+4

class ConnectionModule;

class ConnectionModuleMessageWnd : public CWnd
{
public:
	ConnectionModuleMessageWnd();
	void InitParent(ConnectionModule *mod);

private:
	ConnectionModule *p_mod;

	LRESULT InitThreadData(WPARAM wparam,LPARAM lparam);
	LRESULT StatusReady(WPARAM wparam,LPARAM lparam);
	LRESULT VendorCountsReady(WPARAM wparam,LPARAM lparam);
	LRESULT SpoofDataReady(WPARAM wparam,LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};

#endif // CONNECTION_MODULE_MESSAGE_WND_H