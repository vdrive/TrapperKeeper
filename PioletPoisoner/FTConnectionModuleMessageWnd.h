// FTConnectionModuleMessageWnd.h
#pragma once
#define WM_FT_INIT_THREAD_DATA		WM_USER+1

//#define WM_STATUS_READY			WM_USER+2
//#define WM_VENDOR_COUNTS_READY	WM_USER+3
//#define WM_SPOOF_DATA_READY WM_USER+4

class FTConnectionModule;
class FTConnectionModuleMessageWnd : public CWnd
{
public:

	FTConnectionModuleMessageWnd();
	void InitParent(FTConnectionModule *mod);

private:
	FTConnectionModule *p_mod;

	LRESULT InitFTThreadData(WPARAM wparam,LPARAM lparam);
//	LRESULT StatusReady(WPARAM wparam,LPARAM lparam);
//	LRESULT VendorCountsReady(WPARAM wparam,LPARAM lparam);
//	LRESULT SpoofDataReady(WPARAM wparam,LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};