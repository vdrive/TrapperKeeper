#pragma once
#define WM_TAP	(WM_USER+1)
#define WM_RELOAD_DLLS	(WM_USER+2)

class CTrapperKeeperApp;
class TrapperKeeperWnd : public CWnd
{
public:
	void InitApp(CTrapperKeeperApp *app);

protected:
	LRESULT Tap(WPARAM wparam,LPARAM lparam);
	LRESULT ReloadDlls(WPARAM wparam,LPARAM lparam);

	CTrapperKeeperApp *p_app;

	DECLARE_MESSAGE_MAP()
};
