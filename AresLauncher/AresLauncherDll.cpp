#include "StdAfx.h"
#include "AresLauncherDll.h"

//
//
//
AresLauncherDll::AresLauncherDll(void)
{
}

//
//
//
AresLauncherDll::~AresLauncherDll(void)
{
}

//
//
//
void AresLauncherDll::DllInitialize()
{
	m_dlg.Create(IDD_ARES_DLG,CWnd::GetDesktopWindow());
}

//
//
//
void AresLauncherDll::DllUnInitialize()
{

}

//
//
//
void AresLauncherDll::DllStart()
{
	m_dlg.OnBnClickedKillAres();
	m_dlg.OnBnClickedRunAres();
}

//
//
//
void AresLauncherDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void AresLauncherDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}