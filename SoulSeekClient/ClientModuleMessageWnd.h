// ConnectionModuleMessageWnd.h

#pragma once

#define WM_INIT_THREAD_DATA				WM_USER+1
#define WM_CMOD_LOG_MSG					WM_USER+2
#define WM_CMOD_CLOSE_CONNECTION		WM_USER+3
#define WM_CMOD_CONNECTED				WM_USER+4
#define WM_CMOD_START_TIMER				WM_USER+5
#define WM_CMOD_STOP_TIMER				WM_USER+6
#define WM_CMOD_KILL					WM_USER+7
#define WM_CMOD_WRITE_TO_LOG			WM_USER+8

class ClientModule;

class ClientModuleMessageWnd : public CWnd
{
public:
	ClientModuleMessageWnd();
	~ClientModuleMessageWnd();
	void InitParent(ClientModule *mod);

	LRESULT StartTimer(WPARAM wparam,LPARAM lparam);
	LRESULT StopTimer(WPARAM wparam,LPARAM lparam);
	afx_msg void OnTimer(UINT nIDEvent);

	LRESULT InitThreadData(WPARAM wparam,LPARAM lparam);
	LRESULT LogMsg(WPARAM wparam,LPARAM lparam);
	LRESULT CloseSocket(WPARAM wparam,LPARAM lparam);
	LRESULT Connected(WPARAM wparam,LPARAM lparam);
	LRESULT DeleteThread(WPARAM wparam,LPARAM lparam);
	LRESULT WriteToLog(WPARAM wparam,LPARAM lparam);

	ClientModule *p_mod;

private:
	UINT_PTR m_twentySecondTimer;

	DECLARE_MESSAGE_MAP()
};