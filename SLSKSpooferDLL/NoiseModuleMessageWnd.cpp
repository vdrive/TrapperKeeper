// NoiseModuleMessageWnd.cpp

#include "stdafx.h"
#include "NoiseModuleMessageWnd.h"
#include "NoiseModule.h"

BEGIN_MESSAGE_MAP(NoiseModuleMessageWnd,CWnd)
	ON_MESSAGE(WM_INIT_NOISE_MODULE_THREAD_DATA,InitThreadData)
	ON_MESSAGE(WM_NOISE_MODULE_STATUS_READY,StatusReady)
	ON_WM_TIMER()
END_MESSAGE_MAP()

//
//
//
void NoiseModuleMessageWnd::InitParent(NoiseModule *mod)
{
	p_mod=mod;
}

//
//
//
LRESULT NoiseModuleMessageWnd::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_mod->InitThreadData(wparam,lparam);
	return 0;
}

//
//
//
void NoiseModuleMessageWnd::OnTimer(UINT nIDEvent)
{
	p_mod->OnTimer(nIDEvent);
}

//
//
//
LRESULT NoiseModuleMessageWnd::StatusReady(WPARAM wparam,LPARAM lparam)
{
	p_mod->StatusReady(wparam,lparam);
	return 0;
}