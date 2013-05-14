#include "StdAfx.h"
#include "kazaasupplytakerdll.h"

KazaaSupplyTakerDll::KazaaSupplyTakerDll(void)
{
}

KazaaSupplyTakerDll::~KazaaSupplyTakerDll(void)
{

}

bool KazaaSupplyTakerDll::ReceivedDllData(AppID from_app_id, void* input_data, void* output_data)
//void KazaaSupplyTakerDll::DataReceived(char *source_ip, void *data, UINT data_length)
{
	char *msg = (char *)input_data;

	if (strstr(msg, "Stop Timer") != NULL)
	{
		m_dlg.StopTimer();
	}
	
	if (strstr(msg, "Start Timer") != NULL)
	{
		m_dlg.StartTimer();
	}

	return true;
}

//
//
//
void KazaaSupplyTakerDll::DllInitialize()
{
	m_dlg.Create(IDD_KAZAASUPPLYTAKER_DIALOG, CWnd::GetDesktopWindow());
}

//
//
//
void KazaaSupplyTakerDll::DllUnInitialize()
{
	m_dlg.OnStop();
}

//
//
//
void KazaaSupplyTakerDll::DllStart()
{
	m_dlg.manager.m_com.Register(this, 35);				// For communicating with the DCMaster (Data collecting master)
														//		- receives the projects (project keywords)
														//		- receives the KazaaController IP (to get the supernodes)
														//		- receives the DB connection info, to dump processed data into

//	m_dlg.manager.m_supernode_com.Register(this, 11);	// For communicating with the KazaaController
														//		- request the supernodes
														//		- receive the list

	m_dlg.manager.m_com.InitParent(this);				// Dll Pointers so we can handle the data we receive nicely. :)
//	m_dlg.manager.m_supernode_com.InitParent(this);

	m_dlg.manager.InitParent(&m_dlg);				// initalize with a pointer to the dialog
	m_dlg.manager.m_com.ReadKeywordDataFromFile();	// this loads all known projects from the hard-drive

	// Auto Start occurs after the supernode list has been received, in KazaaManager::SetSuperNodeList()
	m_dlg.Log(0, NULL, "Waiting to receive supernode list...");
}

//
//
//
void KazaaSupplyTakerDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
}
