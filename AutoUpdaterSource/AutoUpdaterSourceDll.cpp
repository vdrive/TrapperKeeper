#include "StdAfx.h"
#include "autoupdatersourcedll.h"
#include "Resource.h"
#include "IO.h"

AutoUpdaterSourceDll::AutoUpdaterSourceDll(void)
{
}

AutoUpdaterSourceDll::~AutoUpdaterSourceDll(void)
{
}

//
// Register() and Init() the Com object.
// Also, set up the GUI list boxes
//
void AutoUpdaterSourceDll::DllStart()
{
	m_AutoUpdaterSourceCom.Register(this, 29);
	m_AutoUpdaterSourceCom.InitParent(this);

	m_dlg.m_list_update_log.InsertColumn(0, "Time", LVCFMT_CENTER, 120, -1);
	m_dlg.m_list_update_log.InsertColumn(1, "Last IP Updated", LVCFMT_CENTER, 120, -1);
	m_dlg.m_list_update_log.InsertColumn(2, "Send Data Return Value", LVCFMT_CENTER, 180, -1);

	// For testing
//	m_AutoUpdaterSourceCom.DataReceived("38.119.66.28", "Gimmie da goodz", strlen("Gimmie da goodz")+1);
}

void AutoUpdaterSourceDll::DllInitialize()
{
	m_dlg.Create(IDD_AutoUpdaterSourceDlg,CWnd::GetDesktopWindow());
}

void AutoUpdaterSourceDll::DllUnInitialize()
{
	// Do nothing...
}

void AutoUpdaterSourceDll::DllShowGUI()
{
	PopulateCurrentDllListBox();
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.UpdateData(TRUE);
}

void AutoUpdaterSourceDll::DataReceived(char *source_name, void *data, int data_length)
{
}

void AutoUpdaterSourceDll::PopulateCurrentDllListBox()
{
	// Delete every other item from the list box.
	int count = m_dlg.m_list_current_plugins.GetCount();
	for (int i=0;i < count;i++)
		m_dlg.m_list_current_plugins.DeleteString(0);
	
	m_dlg.m_list_current_plugins.UpdateData(FALSE);

	_finddata_t foundData;
	intptr_t handle;

	handle = _findfirst("C:\\syncher\\src\\Plug-Ins\\*.dll", &foundData);

	if (foundData.attrib == 32)	// The first DLL
		m_dlg.m_list_current_plugins.InsertString(-1, foundData.name);

	while (_findnext(handle, &foundData)==0) // The other ones
	{
		if (foundData.attrib == 32)
			m_dlg.m_list_current_plugins.InsertString(-1, foundData.name);
	}

	m_dlg.m_list_current_plugins.UpdateData(TRUE);
}