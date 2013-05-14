// SearchManagerMessageWnd.h

#pragma once

#define WM_INIT_THREAD_DATA				WM_USER+1
#define WM_SM_SEND_RESULTS				WM_USER+2
#define WM_SM_START_TIMER				WM_USER+3
#define WM_SM_STOP_TIMER				WM_USER+4
#define WM_SM_UPDATE_GUI				WM_USER+5

class SearchManager;

class SearchManagerMessageWnd : public CWnd
{
public:
	SearchManagerMessageWnd();
	~SearchManagerMessageWnd();
	void InitParent(SearchManager *mod);

	afx_msg void OnTimer(UINT nIDEvent);
	LRESULT StartTimer(WPARAM wparam,LPARAM lparam);
	LRESULT StopTimer(WPARAM wparam,LPARAM lparam);
	LRESULT UpdateGUI(WPARAM wparam,LPARAM lparam);

	LRESULT InitThreadData(WPARAM wparam,LPARAM lparam);
	LRESULT ReturnResults(WPARAM wparam,LPARAM lparam);

	SearchManager *p_mod;

private:
	UINT_PTR m_minuteTimer;
	UINT_PTR m_hourTimer;
	UINT_PTR m_dayTimer;
	DECLARE_MESSAGE_MAP()
};