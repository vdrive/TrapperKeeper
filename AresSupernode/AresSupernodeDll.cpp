#include "StdAfx.h"
#include "AresSupernodeDll.h"
//
//
//
AresSupernodeDll::AresSupernodeDll(void)
{
	mp_system=NULL;
}

//
//
//
AresSupernodeDll::~AresSupernodeDll(void)
{
}

//
//
//
void AresSupernodeDll::DllInitialize()
{
	AfxInitRichEdit();
	m_dlg.Create(IDD_ARESSUPERNODEDLG,CWnd::GetDesktopWindow());
}

//
//
//
void AresSupernodeDll::DllUnInitialize()
{
	if(mp_system){
		mp_system->System()->StopSystem();
		delete mp_system;
	}
	mp_system=NULL;
}

//
//
//
void AresSupernodeDll::DllStart()
{
	if(mp_system){
		mp_system->System()->StopSystem();
		delete mp_system;
	}
	mp_system=new AresSupernodeSystemRef();
	mp_system->System()->StartSystem();
}

//
//
//
void AresSupernodeDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void AresSupernodeDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}