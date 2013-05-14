#include "StdAfx.h"
#include "TKSyncherDll.h"
#include "DllInterface.h"
#include "SyncherDialog.h"

using namespace syncherspace;
//
//
//
TKSyncherDll::TKSyncherDll(void)
{
}

//
//
//
TKSyncherDll::~TKSyncherDll(void)
{
}

//
//
//
void TKSyncherDll::DllInitialize()
{
	//m_dlg.Create(IDD_SamplePlusIn_DIALOG,CWnd::GetDesktopWindow());
	g_syncher_dialog.Create(IDD_SYNCHERDIALOG,CWnd::GetDesktopWindow());
	m_service.SetDLL(this);
	g_syncher_dialog.SetService(&m_service);
	
	m_service.SetComLink(&m_com_link);
	m_com_link.SetService(&m_service);
}

//
//
//
void TKSyncherDll::DllUnInitialize()
{
	m_service.StopSyncherService();
	g_syncher_dialog.DestroyWindow();
}

//
//
//
void TKSyncherDll::DllStart()
{
	m_com_link.Register(this,DllInterface::m_app_id.m_app_id);
	m_service.StartSyncherService();
	m_exec_source.Register(this,"Executables");
	m_exec_source.MapFinishedChanging("");
}

//
//
//
void TKSyncherDll::DllShowGUI()
{
	g_syncher_dialog.ShowWindow(SW_SHOWNORMAL);
	g_syncher_dialog.RefreshSources();
	g_syncher_dialog.BringWindowToTop();
}

bool TKSyncherDll::ReceivedDllData(AppID from_app_id,void* input_data,void* output_data)
{
	return m_service.ReceivedDllData(from_app_id.m_app_id,(byte*)input_data);
}
