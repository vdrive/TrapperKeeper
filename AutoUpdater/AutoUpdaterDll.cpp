#include "StdAfx.h"
#include "AutoUpdaterDll.h"
#include "DllInterface.h"
#include "IO.h"

//
//
//
AutoUpdaterDll::AutoUpdaterDll(void)
{
}

//
//
//
AutoUpdaterDll::~AutoUpdaterDll(void)
{
}

//
//
//
bool AutoUpdaterDll::ReceivedDllData(AppID from_app_id,void* input_data,void* output_data)
{
	return false;
}

//
//
//
void AutoUpdaterDll::DllInitialize()
{
	m_dlg.Create(IDD_AutoUpdater_DIALOG,CWnd::GetDesktopWindow());
}

//
//
void AutoUpdaterDll::DllUnInitialize()
{
}

//
// Register() and Init() the syncher and com objects
//
void AutoUpdaterDll::DllStart()
{
	TRACE("AutoUpdaterDll, DllStart()\n");

	// Don't delete 'good_dll_names.txt', because it will always contain the most recent list of
	// good DLL filenames.  This can't hurt as it is overwritten whenever an update is made.
	// Delete the buffer file so we re-synch the latest DLLs.
//	DeleteFile("C:\\syncher\\internal file buffer\\buffer_index.txt");

	m_AutoSyncher.Register(this, "Plug-Ins");
	m_AutoSyncher.InitParent(this);

	m_AutoCom.Register(this, 29);
	m_AutoCom.InitParent(this);

	m_dlg.InitAutoDll(this);
}

//
//
void AutoUpdaterDll::DllShowGUI()
{
	PopulateCurrentDllListBox();
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.m_list_box.UpdateData(TRUE);
}

void AutoUpdaterDll::PopulateCurrentDllListBox()
{
	// Delete every other item from the list box.
	int count = m_dlg.m_list_box.GetCount();
	for (int i=0;i < count;i++)
		m_dlg.m_list_box.DeleteString(0);

	_finddata_t foundData;
	intptr_t handle;

	handle = _findfirst("C:\\syncher\\rcv\\Plug-Ins\\*.dll", &foundData);

	if (foundData.attrib == 32)	// The first DLL
		m_dlg.m_list_box.InsertString(-1, foundData.name);

	while (_findnext(handle, &foundData)==0) // The other ones
	{
		if (foundData.attrib == 32)
			m_dlg.m_list_box.InsertString(-1, foundData.name);
	}
}

//
//	Receives a list of DLL filenames
//
void AutoUpdaterDll::DataReceived(char *source_name, void *data, int data_length)
{
	TRACE("AutoUpdaterDll recieved some data...\n");
	v_dll_strs.clear();

	// This data is from the AutoUpdaterSource, it must be a list of current DLLs.
	// Record this list in a file? so that our executable UpdateWatcher can delete bad DLLs.
	byte *ptr;
	ptr = (byte *)data;

	UINT num_dlls = (*(UINT *)ptr);
	ptr += sizeof(UINT);

	// Get the DLL names out of the buffer and into a vector
	for (UINT i=0; i < num_dlls; i++)
	{
		char dll_name[256];
		strcpy(dll_name, (const char *)ptr);
		ptr += strlen(dll_name)+1;

		v_dll_strs.push_back(dll_name);
	}

	CStdioFile file;

#ifdef _DEBUG
	if (file.Open("C:\\Documents and Settings\\onsystems\\Desktop\\Trapper Keeper\\good_dll_names.txt", CFile::modeCreate | CFile::modeWrite, NULL) != 0)	// for DEVELOPMENT
#else
	if (file.Open("good_dll_names.txt", CFile::modeCreate | CFile::modeWrite, NULL) != 0)	// FOR RELEASE
#endif
	{
		for (UINT i=0; i < v_dll_strs.size(); i++)
		{
			CString dll_name = v_dll_strs[i].c_str();
			file.WriteString(dll_name);
			file.WriteString("\n");
		}

		file.Close();
	}
	else
	{
		DWORD error = GetLastError();
		MessageBox(NULL, "Error Opening the file: 'good_dll_names.txt'", "File Open Error", MB_OK);
	}

	//
	//	Post a message to the auto-updater watcher executable, to tell it that we are
	//	reloading the dlls.
	//
	UINT WM_RELOAD = RegisterWindowMessage("RELOAD");
	if (WM_RELOAD != 0)
		::PostMessage(HWND_BROADCAST,WM_RELOAD,(WPARAM)0,(LPARAM)0);
	else
		MessageBox(NULL, "RegisterWindowMessage Error", "We got a return of 0, this is not good.", MB_OK);

	// Now send the list to the Dll loader
//	DllInterface::ReloadDlls(v_dll_list);

}

//
// We have new files, use COM to get the "Good" list
//
void AutoUpdaterDll::GetGoodDllsFromSource(char *source_ip)
{
	TRACE("AutoUpdaterDll requesting list of good DLL names...(Gimmie the goodz)\n");
	m_AutoCom.SendReliableData((char *)source_ip, "Gimmie the goodz", sizeof("Gimmie the goodz"));
}