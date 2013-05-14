#include "StdAfx.h"
#include "sampleplugindll.h"
//
//
//
SamplePlugInDll::SamplePlugInDll(void)
{
}

//
//
//
SamplePlugInDll::~SamplePlugInDll(void)
{
}

//
//
//
void SamplePlugInDll::DllInitialize()
{
	m_dlg.Create(IDD_SamplePlusIn_DIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void SamplePlugInDll::DllUnInitialize()
{

}

//
//
//
void SamplePlugInDll::DllStart()
{
	m_syncher.Register(this, "WinMXSupplies");
	m_syncher.InitParent(this);
}

//
//
//
void SamplePlugInDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void SamplePlugInDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

void SamplePlugInDll::SupplySynched(const char* source_ip)
{
}
