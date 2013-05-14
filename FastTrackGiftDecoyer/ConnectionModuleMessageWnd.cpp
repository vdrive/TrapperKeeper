// ConnectionModuleMessageWnd.cpp

#include "stdafx.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModule.h"

BEGIN_MESSAGE_MAP(ConnectionModuleMessageWnd,CWnd)
	ON_MESSAGE(WM_INIT_THREAD_DATA,InitThreadData)
	ON_WM_TIMER()
	ON_MESSAGE(WM_STATUS_READY,StatusReady)
	ON_MESSAGE(WM_VENDOR_COUNTS_READY,VendorCountsReady)
	ON_MESSAGE(WM_LOG_MSG,LogMsg)
END_MESSAGE_MAP()

//
//
//
ConnectionModuleMessageWnd::ConnectionModuleMessageWnd()
{
	p_mod=NULL;
}

//
//
//
void ConnectionModuleMessageWnd::InitParent(ConnectionModule *mod)
{
	p_mod=mod;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_mod->InitThreadData(wparam,lparam);
	return 0;
}

//
//
//
void ConnectionModuleMessageWnd::OnTimer(UINT nIDEvent)
{
	p_mod->OnTimer(nIDEvent);
}

//
//
//
LRESULT ConnectionModuleMessageWnd::StatusReady(WPARAM wparam,LPARAM lparam)
{
	p_mod->StatusReady(wparam,lparam);
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::VendorCountsReady(WPARAM wparam,LPARAM lparam)
{
	p_mod->VendorCountsReady(wparam,lparam);
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::LogMsg(WPARAM wparam,LPARAM lparam)
{
	p_mod->LogMsg(wparam,lparam);
	return 0;
}