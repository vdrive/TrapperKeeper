#include "StdAfx.h"
#include "DCSupplydll.h"
#include "myfiletransferinterface.h"
#include "../TkFileTransfer/tkfiletransferinterface.h"


//
//
//
DCSupplyDll::DCSupplyDll(void)
{
}

//
//
//
DCSupplyDll::~DCSupplyDll(void)
{
}

//
//
//
void DCSupplyDll::DllInitialize()
{
	m_dlg.Create(IDD_DC_SUPPLY_DIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void DCSupplyDll::DllUnInitialize()
{

}

//
//
//
void DCSupplyDll::DllStart()
{
	m_project_manager.DllStart(this,"DCSupply");
	m_dlg.SetProjectManager(m_project_manager);
}

//
//
//
void DCSupplyDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void DCSupplyDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}