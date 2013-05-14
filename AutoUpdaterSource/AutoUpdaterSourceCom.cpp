#include "StdAfx.h"
#include "AutoUpdaterSourceCom.h"
#include "AutoUpdaterSourceDll.h"
#include "IO.h"

typedef AppID(*GetAppIDFuncPtr)();		// for GetVersion() DLL function call
//modified by Ivan: We no longer to call InitDll before calling GetAppID
//typedef bool(*InitDll)(string);
//typedef void(*UnInitDll)();

AutoUpdaterSourceCom::AutoUpdaterSourceCom(void)
{
}

AutoUpdaterSourceCom::~AutoUpdaterSourceCom(void)
{
}

void AutoUpdaterSourceCom::InitParent(AutoUpdaterSourceDll *parent)
{
	p_parent = parent;
}

void AutoUpdaterSourceCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	// Hopefully the message recieved here will be: "Gimmie the goodz"
	// if so, send the DLL list back to the source_ip
	if (strcmp((const char *)data, "Gimmie the goodz") == 0)
	{
		GetDLLs();
/*		
		// Make sure the AutoUpdater, Com, Syncher and File Transfer Dll's are present
		bool got_necessary_dlls;
		got_necessary_dlls = CheckDlls();
		if (!got_necessary_dlls)
		{
			// Insert to the report view list box
			int item_count = p_parent->m_dlg.m_list_update_log.GetItemCount();
			p_parent->m_dlg.m_list_update_log.InsertItem(item_count, "New Item");

			p_parent->m_dlg.m_list_update_log.SetItemText(item_count, 0, CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"));
			p_parent->m_dlg.m_list_update_log.SetItemText(item_count, 1, source_ip);
			p_parent->m_dlg.m_list_update_log.SetItemText(item_count, 2, "** Error ** Need ap ids: {2,61,62,63}");

			return;
		}
*/
		// Now the list of dlls is in v_dll_strs, turn that into a buffer for sending...
		byte *buf;
		UINT data_len = sizeof(UINT);
		
		for (UINT i=0; i < (UINT)v_dll_strs.size(); i++)
		{
			data_len += (UINT)strlen(v_dll_strs[i])+1;
		}
		data_len++;

		buf = new byte[data_len]; // this may need to be a little bigger
		ZeroMemory(buf, data_len);

		byte *ptr = buf;
		UINT num_dlls = (UINT)v_dll_strs.size();
		*((UINT *)ptr) = num_dlls;
		ptr += sizeof(UINT);

		// Build the buffer of data, composed of DLL filenames
		for (int i=0; i < (int)v_dll_strs.size(); i++)
		{
			string dll_name = v_dll_strs[i].GetString();
			strcpy((char *)ptr, dll_name.c_str());
			ptr += strlen(v_dll_strs[i])+1;
		}

		// Send the buffer of data:
		CString return_val;
		bool ret = SendReliableData(source_ip, (void *)buf, data_len);
		if (ret)
			return_val = "Success";
		else
			return_val = "Send Error";
			
		// Insert to the report view list box
		int item_count = p_parent->m_dlg.m_list_update_log.GetItemCount();
		p_parent->m_dlg.m_list_update_log.InsertItem(item_count, "New Item");

		p_parent->m_dlg.m_list_update_log.SetItemText(item_count, 0, CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S"));
		p_parent->m_dlg.m_list_update_log.SetItemText(item_count, 1, source_ip);
		p_parent->m_dlg.m_list_update_log.SetItemText(item_count, 2, return_val);

		delete [] buf;
	}
}

/*
//
// We must have ap Ids of (2,61,62,63)
//
bool AutoUpdaterSourceCom::CheckDlls()
{
	bool found2, found61, found62, found63;
	found2 = false;found61 = false;found62 = false;found63 = false;

	for (UINT i=0; i<v_dll_list.size(); i++)
	{
		if (v_dll_list[i].m_app_id == 2)
			found2=true;
		if (v_dll_list[i].m_app_id == 61)
			found61=true;
		if (v_dll_list[i].m_app_id == 62)
			found62=true;
		if (v_dll_list[i].m_app_id == 63)
			found63=true;
	}

	if (found2 && found61 && found62 && found63)
		return true;
	else
		return false;
}
*/

void AutoUpdaterSourceCom::GetDLLs()
{
//	v_dll_list.clear();
	v_dll_strs.clear();

	_finddata_t foundData;
	intptr_t handle;

	handle = _findfirst("C:\\syncher\\src\\Plug-Ins\\*.dll", &foundData);

	if (foundData.attrib == 32)	// The first DLL
	{	
		CString dll_path = "C:\\syncher\\src\\Plug-Ins\\";
		dll_path += foundData.name;

/*
		HINSTANCE hInst = LoadLibrary(dll_path);
		ASSERT(hInst != NULL);

		GetAppIDFuncPtr pFunction = (GetAppIDFuncPtr)GetProcAddress(hInst, "GetAppID");
		ASSERT(pFunction != NULL);

		AppID newDll;
		newDll = (*pFunction)();
*/
		CString dll_name = foundData.name;

		v_dll_strs.push_back(dll_name);
//		v_dll_list.push_back(newDll);

//		FreeLibrary(hInst);
	}

	while (_findnext(handle, &foundData)==0) // There are more dlls...
	{
		if (foundData.attrib == 32)
		{	
			CString dll_path = "C:\\syncher\\src\\Plug-Ins\\";
			dll_path += foundData.name;
/*
			HINSTANCE hInst = LoadLibrary(dll_path);
			ASSERT(hInst != NULL);

			GetAppIDFuncPtr pFunction = (GetAppIDFuncPtr)GetProcAddress(hInst, "GetAppID");
			ASSERT(pFunction != NULL);

			AppID newDll;
			newDll = (*pFunction)();
*/
			CString dll_name;
			dll_name.Empty();
			dll_name = foundData.name;

			v_dll_strs.push_back(dll_name);
//			v_dll_list.push_back(newDll);

//			FreeLibrary(hInst);
		}
	}
}