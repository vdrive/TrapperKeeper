#include "StdAfx.h"
#include "BTinterdictordll.h"
//
//
//
BTinterdictorDll::BTinterdictorDll(void)
{
}

//
//
//
BTinterdictorDll::~BTinterdictorDll(void)
{
}

//
//
//
void BTinterdictorDll::DllInitialize()
{
	m_dlg.Create(IDD_SamplePlusIn_DIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void BTinterdictorDll::DllUnInitialize()
{

}

//
//
//
void BTinterdictorDll::DllStart()
{
	//m_syncher.Register(this, "WinMXSupplies");
//	m_syncher.InitParent(this);
}

//
//
//
void BTinterdictorDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void BTinterdictorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

/*
void BTinterdictorDll::SupplySynched(const char* source_ip)
{
}
*/
