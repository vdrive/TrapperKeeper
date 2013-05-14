#include "StdAfx.h"
#include "DCScannerdll.h"
#include "../NameServer/nameserverinterface.h"
#include "mynameserverinterface.h"
#include "myfiletransferinterface.h"
#include "../TkFileTransfer/tkfiletransferinterface.h"


//
//
//
DCScannerDll::DCScannerDll(void)
{
}

//
//
//
DCScannerDll::~DCScannerDll(void)
{
}

//
//
//
void DCScannerDll::DllInitialize()
{
	m_dlg.Create(IDD_DC_SCANNER_DIALOG,CWnd::GetDesktopWindow());
}

//
//
//
void DCScannerDll::DllUnInitialize()
{

}

//
//
//
void DCScannerDll::DllStart()
{
//	NameServerInterface temp;
	//vector<RackInfo> racks;
	//temp.RequestIPeX(racks);
	//temp.RequestIPeX(rack_data);
//	vector<string> v_temp;
//	temp.RequestIP("asdf",v_temp);
//	TRACE("%d",v_temp.size());
//	vector<string> names_temp;
//	temp.RequestAllNames(names_temp);

	m_project_manager.DllStart(this,"DCScanner");
	m_dlg.SetProjectManager(m_project_manager);
}

//
//
//
void DCScannerDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void DCScannerDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}