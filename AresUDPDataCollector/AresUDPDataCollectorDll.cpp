#include "StdAfx.h"
#include "AresUDPDataCollectorDll.h"
//
//
//
AresUDPDataCollectorDll::AresUDPDataCollectorDll(void)
{
	mp_system=NULL;
}

//
//
//
AresUDPDataCollectorDll::~AresUDPDataCollectorDll(void)
{
}

//
//
//
void AresUDPDataCollectorDll::DllInitialize()
{
//	m_dlg.Create(IDD_SamplePlusIn_DIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void AresUDPDataCollectorDll::DllUnInitialize()
{
	if(mp_system!=NULL){
		mp_system->System()->StopSystem();
		delete mp_system;
		mp_system=NULL;
	}
}

//
//
//
void AresUDPDataCollectorDll::DllStart()
{
	if(mp_system!=NULL){
		mp_system->System()->StopSystem();
		delete mp_system;
		mp_system=NULL;
	}
	mp_system=new SystemRef();
	mp_system->System()->StartSystem();
}

//
//
//
void AresUDPDataCollectorDll::DllShowGUI()
{
//	m_dlg.ShowWindow(SW_NORMAL);
//	m_dlg.BringWindowToTop();
}

//
//
//
void AresUDPDataCollectorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}