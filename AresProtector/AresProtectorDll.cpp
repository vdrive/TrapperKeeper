#include "StdAfx.h"
#include "AresProtectorDll.h"

//
//
//
AresProtectorDll::AresProtectorDll(void)
{
	mp_ref=NULL;
}

//
//
//
AresProtectorDll::~AresProtectorDll(void)
{
}

//
//
//
void AresProtectorDll::DllInitialize()
{
	TRACE("AresProtectorDll::DllInitialize() BEGIN\n");
	m_dlg.Create(IDD_ARESDIALOG,CWnd::GetDesktopWindow());
	TRACE("AresProtectorDll::DllInitialize() END\n");
}

//
//
//
void AresProtectorDll::DllUnInitialize()
{
	TRACE("AresProtectorDll::DllUnInitialize() BEGIN\n");
	if(mp_ref){
		mp_ref->System()->StopSystem();
		delete mp_ref;
	}
	mp_ref=NULL;
	TRACE("AresProtectorDll::DllUnInitialize() END\n");
}

//
//
//
void AresProtectorDll::DllStart()
{
	if(mp_ref==NULL){
		mp_ref=new AresProtectionSystemReference();
		mp_ref->System()->StartSystem();
	}
}

//
//
//
void AresProtectorDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void AresProtectorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}