#include "StdAfx.h"
#include "KademliaProtectordll.h"

//
//
//
KademliaProtectorDll::KademliaProtectorDll(void)
{
	mp_system=NULL;
}

//
//
//
KademliaProtectorDll::~KademliaProtectorDll(void)
{
}

//
//
//
void KademliaProtectorDll::DllInitialize()
{
	m_dlg.Create(IDD_KADEMLIAPROTECTORDIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void KademliaProtectorDll::DllUnInitialize()
{
	if(mp_system){
		mp_system->System()->StopSystem();
		delete mp_system;
		mp_system=NULL;
	}
}

//
//
//
void KademliaProtectorDll::DllStart()
{
	if(mp_system){
		mp_system->System()->StopSystem();
		delete mp_system;
	}
	mp_system=new KademliaProtectorSystemReference();
	mp_system->System()->StartSystem();
}

//
//
//
void KademliaProtectorDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void KademliaProtectorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}