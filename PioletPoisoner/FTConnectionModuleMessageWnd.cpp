// ConnectionModuleMessageWnd.cpp

#include "stdafx.h"
#include "FTConnectionModuleMessageWnd.h"
#include "FTConnectionModule.h"

BEGIN_MESSAGE_MAP(FTConnectionModuleMessageWnd,CWnd)
	ON_MESSAGE(WM_FT_INIT_THREAD_DATA,InitFTThreadData)
//	ON_MESSAGE(WM_STATUS_READY,StatusReady)
//	ON_MESSAGE(WM_VENDOR_COUNTS_READY,VendorCountsReady)
//	ON_MESSAGE(WM_SPOOF_DATA_READY,SpoofDataReady)
END_MESSAGE_MAP()

//
//
//
FTConnectionModuleMessageWnd::FTConnectionModuleMessageWnd()
{
	p_mod=NULL;
}

//
//
//
void FTConnectionModuleMessageWnd::InitParent(FTConnectionModule *mod)
{
	p_mod=mod;
}

//
//
//
LRESULT FTConnectionModuleMessageWnd::InitFTThreadData(WPARAM wparam,LPARAM lparam)
{
	p_mod->InitThreadData(wparam,lparam);
	return 0;
}

//
//
//
/*
LRESULT ConnectionModuleMessageWnd::StatusReady(WPARAM wparam,LPARAM lparam)
{
	p_mod->StatusReady(wparam,lparam);
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::SpoofDataReady(WPARAM wparam,LPARAM lparam)
{
	p_mod->SpoofDataReady();
	return 0;
}

//
//
//
LRESULT ConnectionModuleMessageWnd::VendorCountsReady(WPARAM wparam,LPARAM lparam)
{
//	p_mod->VendorCountsReady(wparam,lparam);
	return 0;
}
*/