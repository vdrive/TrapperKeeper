#include "StdAfx.h"
#include "networkmonitordll.h"

//
//
//
NetworkMonitorDll::NetworkMonitorDll(void)
{
}

//
//
//
NetworkMonitorDll::~NetworkMonitorDll(void)
{
}

//
//
//
void NetworkMonitorDll::DllInitialize()
{
	m_dlg.Create(IDD_NETWORKMONITORDIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void NetworkMonitorDll::DllUnInitialize()
{
	m_network_system.Shutdown();
}

//
//
//
void NetworkMonitorDll::DllStart()
{
	m_network_system.Init();
	m_dlg.SetNetworkSystem(&m_network_system);
}

//
//
//
void NetworkMonitorDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void NetworkMonitorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}
