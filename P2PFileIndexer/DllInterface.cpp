// DllInterface.cpp : Defines the initialization routines for the DLL.

#include "StdAfx.h"
#include "dllinterface.h"
//#include "../DllLoader/AppIDs.h"

//// Static members declaration
HINSTANCE DllInterface::m_hinst;
AppID DllInterface::m_app_id;

//Change it to your main app class instead of PlugInApp class here
P2PFileIndexerDll DllInterface::m_the_app;

//
//
//
DllInterface::DllInterface(void)
{
}

//
//
//
DllInterface::~DllInterface(void)
{
}

//
//
//
bool DllInterface::InitDll(string path)
{
	m_hinst = LoadLibrary(path.c_str());
	m_the_app.DllInitialize();
	if(m_hinst != NULL)
		return true;
	else
		return false;
}

//
//
//
void DllInterface::UnInitialize()
{
	FreeLibrary(m_hinst);
	m_the_app.DllUnInitialize();
}

//
//
//
bool DllInterface::ReceivedData(AppID from_app_id, void* input_data, void* output_data)
{
	bool ret = m_the_app.DllReceivedData(from_app_id, input_data, output_data);
	return ret;
	//Just ignore it for now
	//if(!ret)
		//just send it back without error code for now
	//	SendData(from_app_id,data,datasize);
}

//
//
//
AppID DllInterface::GetAppID()
{
	//setting up your appID
	m_app_id.m_app_id = 501;
	m_app_id.m_version = 0x00000007;
	m_app_id.m_app_name = "P2P File Indexing System";

	return m_app_id;
}

//
//
//
void DllInterface::Start()
{
	m_the_app.DllStart();
}

//
//
//
void DllInterface::ShowGUI()
{
	m_the_app.DllShowGUI();
}

//
//
//

bool DllInterface::SendData(AppID to_app_id, void* input_data,  void* output_data)
{
	typedef bool (*MYPROC)(AppID, AppID, void*, void*);
	MYPROC ProcAdd;

	ProcAdd = (MYPROC) GetProcAddress(m_hinst, "DllManagerSendData");

	return (ProcAdd)(m_app_id, to_app_id, input_data, output_data);
}

//
//
//
void DllInterface::ReloadDlls(vector<AppID>& apps)
{
	typedef void (*MYPROC)(vector<AppID>);
	MYPROC ProcAdd;

	ProcAdd = (MYPROC) GetProcAddress(m_hinst, "DllManagerReloadDlls");

	(ProcAdd)(apps);

/*	CWnd *wnd_ptr=NULL;
	wnd_ptr=CWnd::FindWindow("Trapper Keeper V1.x",NULL);
	if(wnd_ptr!=NULL)
	{
		AppIDs* app_ids;
		app_ids = new AppIDs;
		app_ids->v_app_ids = apps;
		
		HWND hwnd=wnd_ptr->GetSafeHwnd();
		BOOL ret=::PostMessage(hwnd,WM_USER+2,(WPARAM)app_ids,(LPARAM)0);
	}
*/
}
