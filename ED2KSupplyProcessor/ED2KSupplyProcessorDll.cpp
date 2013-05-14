#include "StdAfx.h"
#include "ED2KSupplyProcessorDll.h"

//
//
//
ED2KSupplyProcessorDll::ED2KSupplyProcessorDll(void)
{
}

//
//
//
ED2KSupplyProcessorDll::~ED2KSupplyProcessorDll(void)
{
}

//
//
//
void ED2KSupplyProcessorDll::DllInitialize()
{
//	m_dlg.Create(IDD_SamplePlusIn_DIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void ED2KSupplyProcessorDll::DllUnInitialize()
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
void ED2KSupplyProcessorDll::DllStart()
{
	if(mp_system) delete mp_system;
	mp_system=new SupplySystemRef();
	mp_system->System()->StartSystem(this);
}

//
//
//
void ED2KSupplyProcessorDll::DllShowGUI()
{
//	m_dlg.ShowWindow(SW_NORMAL);
//	m_dlg.BringWindowToTop();
}

//
//
//
void ED2KSupplyProcessorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}