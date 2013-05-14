// ConnectionModuleMessageWnd.cpp

#include "stdafx.h"
#include "ConnectionModuleMessageWnd.h"
#include "ConnectionModule.h"
#include "ClientData.h"

BEGIN_MESSAGE_MAP(ConnectionModuleMessageWnd,CWnd)
	ON_MESSAGE(WM_INIT_THREAD_DATA,InitThreadData)
	ON_WM_TIMER()
	ON_MESSAGE(WM_STATUS_READY,StatusReady)
	ON_MESSAGE(WM_VENDOR_COUNTS_READY,VendorCountsReady)
	ON_MESSAGE(WM_LOG_MSG,LogMsg)
	ON_MESSAGE(WM_KILL_MODULE,KillMsg)
	ON_MESSAGE(WM_REPORT_DATA,ReportData)
	ON_MESSAGE(WM_REPORT_MOD_DATA,ReportModData)
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

LRESULT ConnectionModuleMessageWnd::KillMsg(WPARAM wparam,LPARAM lparam)
{
	p_mod->KillYourself();
	return(0);
}

LRESULT ConnectionModuleMessageWnd::ReportData(WPARAM wparam,LPARAM lparam)
{
	p_mod->DataReported(wparam,lparam);
	return(0);
}

LRESULT ConnectionModuleMessageWnd::ReportModData(WPARAM wparam,LPARAM lparam)
{
	p_mod->ModuleDataReported(wparam,lparam);
	return(0);
}