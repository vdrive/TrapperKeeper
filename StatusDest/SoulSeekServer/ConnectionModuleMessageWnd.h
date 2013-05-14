// ConnectionModuleMessageWnd.h

#pragma once

#define WM_INIT_THREAD_DATA				WM_USER+1
#define WM_CMOD_LOG_MSG					WM_USER+2
#define WM_CMOD_CLOSE_CONNECTION		WM_USER+3
#define WM_CMOD_RECEIVED_CONNECTION		WM_USER+4
#define WM_CMOD_SEARCH_REQUEST		WM_USER+5

class ConnectionModule;

class ConnectionModuleMessageWnd : public CWnd
{
public:
	ConnectionModuleMessageWnd();
	~ConnectionModuleMessageWnd();
	void InitParent(ConnectionModule *mod);

	LRESULT InitThreadData(WPARAM wparam,LPARAM lparam);
	LRESULT LogMsg(WPARAM wparam,LPARAM lparam);
	LRESULT CloseSocket(WPARAM wparam,LPARAM lparam);
	LRESULT ReceivedConnection(WPARAM wparam,LPARAM lparam);
	LRESULT ProcessSearchRequest(WPARAM wparam,LPARAM lparam);

	ConnectionModule *p_mod;

private:

	DECLARE_MESSAGE_MAP()
};