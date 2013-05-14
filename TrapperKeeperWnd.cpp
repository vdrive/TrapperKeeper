#include "StdAfx.h"
#include "TrapperKeeperWnd.h"
#include "TrapperKeeper.h"

BEGIN_MESSAGE_MAP(TrapperKeeperWnd,CWnd)
	ON_MESSAGE(WM_TAP,Tap)
	ON_MESSAGE(WM_RELOAD_DLLS,ReloadDlls)
END_MESSAGE_MAP()

//
//
//
void TrapperKeeperWnd::InitApp(CTrapperKeeperApp *app)
{
	p_app=app;
}

//
//
//
LRESULT TrapperKeeperWnd::Tap(WPARAM wparam,LPARAM lparam)
{
	p_app->Tap();
	return 0;
}

//
//
//
LRESULT TrapperKeeperWnd::ReloadDlls(WPARAM wparam,LPARAM lparam)
{
	p_app->ReloadDlls(wparam, lparam);
	return 0;
}