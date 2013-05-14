#include "StdAfx.h"
#include "OverNetLauncherDll.h"

//
//
//
OverNetLauncherDll::OverNetLauncherDll(void)
{
}

//
//
//
OverNetLauncherDll::~OverNetLauncherDll(void)
{
}

//
//
//
void OverNetLauncherDll::DllInitialize()
{
	m_dlg.Create(IDD_OVERNET_LAUNCHER_DLG,CWnd::GetDesktopWindow());
}

//
//
//
void OverNetLauncherDll::DllUnInitialize()
{
	m_dlg.KillClients();
}

//
//
//
void OverNetLauncherDll::DllStart()
{
	m_dlg.SetTimer(1, 1000, NULL);
}

//
//
//
void OverNetLauncherDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void OverNetLauncherDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}