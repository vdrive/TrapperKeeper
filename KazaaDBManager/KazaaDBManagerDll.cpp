#include "StdAfx.h"
#include "KazaaDBManagerdll.h"

//
//
//
KazaaDBManagerDll::KazaaDBManagerDll(void)
{
}

//
//
//
KazaaDBManagerDll::~KazaaDBManagerDll(void)
{
}

//
//
//
void KazaaDBManagerDll::DllInitialize()
{
	m_dlg.Create(IDD_KAZAADBMANAGERDIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void KazaaDBManagerDll::DllUnInitialize()
{
	m_dlg.StopSystem();
}

//
//
//
void KazaaDBManagerDll::DllStart()
{
	this->Register(this,"KazaaDBManager");
}

//
//
//
void KazaaDBManagerDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void KazaaDBManagerDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

//a method inherited from the syncher interface that this class is subclassed from
void KazaaDBManagerDll::MapFinishedChanging(const char *source_ip){

}