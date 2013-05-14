// ConnectionModuleMessageWnd.h

#ifndef CONNECTION_MODULE_MESSAGE_WND_H
#define CONNECTION_MODULE_MESSAGE_WND_H

#define WM_INIT_THREAD_DATA		WM_USER+1
#define WM_STATUS_READY			WM_USER+2
#define WM_LOG					WM_USER+3

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
	LRESULT Log(WPARAM wparam,LPARAM lparam);
	DECLARE_MESSAGE_MAP()
};

#endif // CONNECTION_MODULE_MESSAGE_WND_H