#include "StdAfx.h"
#include "GnutellaFileDownloaderDll.h"
//
//
//
GnutellaFileDownloaderDll::GnutellaFileDownloaderDll(void)
{
}

//
//
//
GnutellaFileDownloaderDll::~GnutellaFileDownloaderDll(void)
{
}

//
//
//
void GnutellaFileDownloaderDll::DllInitialize()
{
	m_dlg.Create(m_dialog,CWnd::GetDesktopWindow());
}

//
//
//
void GnutellaFileDownloaderDll::DllUnInitialize()
{

}

//
//
//
void GnutellaFileDownloaderDll::DllStart()
{
//	m_syncher.Register(this, "WinMXSupplies");
//	m_syncher.InitParent(this);
}

//
//
//
void GnutellaFileDownloaderDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void GnutellaFileDownloaderDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

void GnutellaFileDownloaderDll::SupplySynched(const char* source_ip)
{
}
