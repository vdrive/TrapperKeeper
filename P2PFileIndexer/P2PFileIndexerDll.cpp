#include "StdAfx.h"
#include "P2PFileIndexerDll.h"

//
//
//
P2PFileIndexerDll::P2PFileIndexerDll(void)
{
}

//
//
//
P2PFileIndexerDll::~P2PFileIndexerDll(void)
{
}

//
//
//
void P2PFileIndexerDll::DllInitialize()
{
	//m_dlg.Create(IDD_SamplePlusIn_DIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void P2PFileIndexerDll::DllUnInitialize()
{

}

//
//
//
void P2PFileIndexerDll::DllStart()
{
	m_dlg.SetDLL(this);
	m_dlg.Create(IDD_P2PDIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void P2PFileIndexerDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void P2PFileIndexerDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}