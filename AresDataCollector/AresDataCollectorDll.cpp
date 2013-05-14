#include "StdAfx.h"
#include "AresDataCollectorDll.h"
//
//
//
AresDataCollectorDll::AresDataCollectorDll(void)
{
	mp_system=NULL;
}

//
//
//
AresDataCollectorDll::~AresDataCollectorDll(void)
{
}

//
//
//
void AresDataCollectorDll::DllInitialize()
{
	m_dlg.Create(IDD_ARESDCDIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void AresDataCollectorDll::DllUnInitialize()
{
	if(mp_system!=NULL){
		mp_system->System()->StopSystem();
		delete mp_system;
	}
	mp_system=NULL;
}

//
//
//
void AresDataCollectorDll::DllStart()
{
	if(mp_system!=NULL)
		delete mp_system;

	mp_system=new AresDataCollectorSystemReference();
	mp_system->System()->StartSystem();
}

//
//
//
void AresDataCollectorDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void AresDataCollectorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}