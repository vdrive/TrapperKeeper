#include "StdAfx.h"
#include "dllmanager.h"
#include "DllLoaderApp.h"
#include <io.h>

vector<DllInfo> DllManager::v_dlls;
int DllManager::m_num_dlls;
DllLoaderApp* DllManager::p_parent;
vector<AppID> DllManager::v_reload_apps;

//
//
//
DllManager::DllManager(void)
{
	m_num_dlls = 0;
}

//
//
//
DllManager::~DllManager(void)
{
}

//
//
//
void DllManager::LoadAllDlls(void)
{
	char curr_path[256+1];	
	::ZeroMemory(curr_path, 256+1);
	::GetModuleFileName( NULL, curr_path, 256+1);
	int length = (int)strlen(curr_path);
	int count = 0;
	char* iterator = curr_path + length;
	do
	{
		count++;
		iterator--;
	}
	while (iterator > curr_path && *iterator != '\\');
	length = length - count;
	//_ASSERT(length > 0);
	char plugin_dir[256+1];
	char exe_dir[256+1];
	::ZeroMemory(exe_dir, 256+1);
	strncpy(exe_dir, curr_path, length);
	strcat(exe_dir, "\\DllLoader.dll");
	string dllloader_path;
	dllloader_path = exe_dir;
	strcpy(plugin_dir, "C:\\syncher\\rcv\\PLUG-INs\\*.dll");

	struct _finddata_t finddata;
	intptr_t hFile;
	int returned=0;

	char DllFileName[256+1];
	
	hFile=_findfirst(plugin_dir,&finddata);	// get first file

	//add all Dlls filename into DllFileList
	if(hFile != -1)
	{
		int path_length = (int)strlen(plugin_dir);
		while(returned!=-1)
		{
			::ZeroMemory(DllFileName, 256);
			strncpy(DllFileName, plugin_dir, path_length - strlen("*.dll") );
			strcat(DllFileName, finddata.name);
			
			
			//load each Dll module and check for its validity	
			HINSTANCE hinst = LoadLibrary(DllFileName);
			if(hinst != NULL)
			{
				bool valid_dll = CheckForValidDll(hinst);
				if(valid_dll)
				{
					typedef bool (*INIT)(string);
					INIT Init;
					Init = (INIT) GetProcAddress(hinst, "InitDll");
					if(Init != NULL)
					{
						valid_dll =  (Init)(dllloader_path); //Call InitDll
					}
					if(valid_dll)
					{
						DllInfo dll_info = GetInfoFromDll(hinst);
						if(IsDuplicateDll(dll_info)==false)
						{
							v_dlls.push_back(dll_info);
							m_num_dlls++;
						}
					}
					else
					{
						typedef void (*FREE)();
						FREE FreeDll;
						FreeDll = (FREE) GetProcAddress(hinst, "UnInitialize");
						(FreeDll)();
						FreeLibrary(hinst);
					}
				}
				else
					FreeLibrary(hinst);
			}	
						
			returned=_findnext(hFile,&finddata);
		}
	}
	_findclose(hFile);

	StartAllDlls();
}

//
//
//
bool DllManager::CheckForValidDll(HINSTANCE hinst)
{
/*	char* curr_path = new char[256+1];
	::GetModuleFileName( NULL, curr_path, 256+1);
	
	int length = (int)strlen(curr_path);
	int count = 0;
	char* iterator = curr_path + length;
	do
	{
		count++;
		iterator--;
	}
	while (iterator > curr_path && *iterator != '\\');
	length = length - count;
	//_ASSERT(length > 0);
	char exe_dir[256+1];
	::ZeroMemory(exe_dir, 256+1);
	strncpy(exe_dir, curr_path, length);
	strcat(exe_dir, "\\DllLoader.dll");
	delete curr_path;

	string temp;
	temp = exe_dir;
*/
	typedef bool (*INIT)(string);
	typedef AppID (*APPID)();
	typedef void (*UNINTI)();
	typedef void (*START)();
	typedef void (*RECVDATA)(AppID, void*, UINT);
	typedef void (*SHOWGUI)();
	
	INIT Init;
	APPID GetAppID;
	UNINTI UnInitialize;
	START Start;
	RECVDATA ReceivedData;
	SHOWGUI ShowGUI;

	GetAppID = (APPID) GetProcAddress(hinst, "GetAppID");
	UnInitialize = (UNINTI) GetProcAddress(hinst, "UnInitialize");
	Start = (START) GetProcAddress(hinst, "Start");
	ReceivedData = (RECVDATA) GetProcAddress(hinst, "ReceivedData");
	ShowGUI = (SHOWGUI) GetProcAddress(hinst, "ShowGUI");
	Init = (INIT) GetProcAddress(hinst, "InitDll");
	bool valid = ( (Init != NULL) && (GetAppID != NULL) && (UnInitialize !=NULL) 
					 && (Start != NULL)&& (ReceivedData != NULL) && (ShowGUI != NULL));
	/*
	if(valid == true)
	{
		Init = (INIT) GetProcAddress(hinst, "InitDll");
		if(Init != NULL)
		{
			return (Init)(temp); //Call InitApp
		}
	}
	return false;
	*/
	return valid;
}

//
//
//
DllInfo DllManager::GetInfoFromDll(HINSTANCE hinst)
{
	DllInfo dll;
	dll.m_appID = ReturnAppID(hinst);
	dll.m_hinstance = hinst;
	char module_path[256+1];
	::GetModuleFileName(hinst,module_path, sizeof(module_path));
	dll.m_module_path = module_path;
	return dll;
}

//
//
//
AppID DllManager::ReturnAppID(HINSTANCE hinst)
{
	AppID app_id;

	typedef AppID (*APPID)();
	APPID GetAppID;

	GetAppID = (APPID) GetProcAddress(hinst, "GetAppID");
	
	app_id = (GetAppID)();	

	return app_id;
}

//
//
//
void DllManager::Start(AppID& app_id)
{
	typedef void (*MYPROC)();
	MYPROC	Function;
	HINSTANCE hinst;
	vector<DllInfo>::iterator app_iter=v_dlls.begin();
	while(app_iter!=v_dlls.end())
	{
		if(app_iter->m_appID.IsEqual(app_id))
		{
			hinst = app_iter->m_hinstance;
			Function = (MYPROC) GetProcAddress(hinst, "Start");
			if(Function != NULL)
			{
 				(Function)();
				break;
			}
		}
		app_iter++;
	}
}

//
//
//
bool DllManager::ReceivedData(AppID from_app_id, AppID to_app_id, void* input_data, void* output_data)
{
	typedef bool (*MYPROC)(AppID,void*,void*);
	MYPROC	Function;
	HINSTANCE hinst;
	vector<DllInfo>::iterator app_iter=v_dlls.begin();
	while(app_iter!=v_dlls.end())
	{
		if(app_iter->m_appID.IsEqual(to_app_id))
		{
			hinst = app_iter->m_hinstance;
			Function = (MYPROC) GetProcAddress(hinst, "ReceivedData");
			if(Function != NULL)
			{
 				return (Function)(from_app_id,input_data,output_data);
				break;
			}
		}
		app_iter++;
	}
	return false;
}

//
//
//
void DllManager::ShowGUI(AppID& app_id)
{
	typedef void (*MYPROC)();
	MYPROC	Function;
	HINSTANCE hinst;
	vector<DllInfo>::iterator app_iter=v_dlls.begin();
	while(app_iter!=v_dlls.end())
	{
		if(app_iter->m_appID.IsEqual(app_id))
		{
			hinst = app_iter->m_hinstance;
			Function = (MYPROC) GetProcAddress(hinst, "ShowGUI");
			if(Function != NULL)
			{
 				(Function)();
				break;
			}
		}
		app_iter++;
	}
}

//
//
//
bool DllManager::DllManagerSendData(AppID from_app_id, AppID to_app_id,void* input_data,void* output_data)
{
	return ReceivedData(from_app_id, to_app_id, input_data, output_data);
}

//
//
//
void DllManager::DllManagerReloadDlls(vector<AppID>apps)
{
	//ReloadDlls(apps);
	v_reload_apps.clear();
	v_reload_apps = apps;

	//post a message to Trapper Keeper to actually reload dlls
	CWnd *wnd_ptr=NULL;
	wnd_ptr=CWnd::FindWindow("Trapper Keeper V1.x",NULL);
	if(wnd_ptr!=NULL)
	{
		HWND hwnd=wnd_ptr->GetSafeHwnd();
		BOOL ret=::PostMessage(hwnd,WM_USER+2,(WPARAM)0,(LPARAM)0);
	}
}

//
//
//
// UnInitialize the dll and remove it from the manager
void DllManager::UnInitialize(AppID& app_id)
{
	typedef void (*FREE)();
	FREE FreeDll;

	vector<DllInfo>::iterator app_iter=v_dlls.begin();
	while(app_iter!=v_dlls.end())
	{
		if(app_iter->m_appID.IsEqual(app_id))
		{
			FreeDll = (FREE) GetProcAddress(app_iter->m_hinstance, "UnInitialize");
			(FreeDll)();

			FreeLibrary(app_iter->m_hinstance);
			m_num_dlls--;
#ifdef DLL_DEBUG
			/* Temp debug message */
			
			char msg[256];
			sprintf(msg, "Unloaded the dll \"%s\", AppID: %d",app_iter->m_appID.m_app_name.c_str(), app_iter->m_appID.m_app_id);
			MessageBox(NULL,msg,"Trapper Keeper",MB_OK|MB_ICONINFORMATION);
#endif
			v_dlls.erase(app_iter);
			break;
		}
		app_iter++;
	}
}

//
//
// UnInitialize the dll only
void DllManager::UnInitializeDllOnly(DllInfo& info)
{
	typedef void (*FREE)();
	FREE FreeDll;

	FreeDll = (FREE) GetProcAddress(info.m_hinstance, "UnInitialize");
	(FreeDll)();
	FreeLibrary(info.m_hinstance);
#ifdef DLL_DEBUG
	/* Temp debug message */
	char msg[256];
	sprintf(msg, "Unloaded the dll \"%s\", AppID: %d",info.m_appID.m_app_name.c_str(), info.m_appID.m_app_id);
	MessageBox(NULL,msg,"Trapper Keeper",MB_OK|MB_ICONINFORMATION);
#endif
}
//
//
//
void DllManager::FreeAllDlls(void)
{
	typedef void (*FREE)();
	FREE FreeDll;

	vector<DllInfo>::iterator app_iter=v_dlls.begin();
	while(app_iter!=v_dlls.end())
	{
		FreeDll = (FREE) GetProcAddress(app_iter->m_hinstance, "UnInitialize");
		(FreeDll)();

		FreeLibrary(app_iter->m_hinstance);
		m_num_dlls--;
#ifdef DLL_DEBUG
		/* Temp debug message */
		char msg[256];
		sprintf(msg, "Unloaded the dll \"%s\", AppID: %d",app_iter->m_appID.m_app_name.c_str(), app_iter->m_appID.m_app_id);
		MessageBox(NULL,msg,"Trapper Keeper",MB_OK|MB_ICONINFORMATION);
#endif
		app_iter++;
	}
}

//
//
//
void DllManager::ShowGUI(vector<UINT>& app_ids)
{
	for(unsigned int i=0; i<app_ids.size(); i++)
	{
		for(unsigned int j=0; j<v_dlls.size(); j++)
		{
			if(v_dlls[j].m_appID.m_app_id  == app_ids[i])
			{
				ShowGUI(v_dlls[j].m_appID);
				break;
			}
		}
	}
}

//
//
//
void DllManager::ShowGUI(UINT& app_id)
{
	for(unsigned int j=0; j<v_dlls.size(); j++)
	{
		if(v_dlls[j].m_appID.m_app_id  == app_id)
		{
			ShowGUI(v_dlls[j].m_appID);
			break;
		}
	}
}

//
//
//
void DllManager::LoadDlls(vector<AppID>& app_ids)
{
	char curr_path[256+1];	
	::ZeroMemory(curr_path, 256+1);
	::GetModuleFileName( NULL, curr_path, 256+1);
	int length = (int)strlen(curr_path);
	int count = 0;
	char* iterator = curr_path + length;
	do
	{
		count++;
		iterator--;
	}
	while (iterator > curr_path && *iterator != '\\');
	length = length - count;
	//_ASSERT(length > 0);
	char plugin_dir[256+1];
	char exe_dir[256+1];
	::ZeroMemory(exe_dir, 256+1);
	strncpy(exe_dir, curr_path, length);
	strcat(exe_dir, "\\DllLoader.dll");
	string dllloader_path;
	dllloader_path = exe_dir;
	strcpy(plugin_dir, "C:\\syncher\\rcv\\PLUG-INs\\*.dll");

	struct _finddata_t finddata;
	intptr_t hFile;
	int returned=0;

	char DllFileName[256+1];
	
	hFile=_findfirst(plugin_dir,&finddata);	// get first file

	//add all Dlls filename into DllFileList
	if(hFile != -1)
	{
		int path_length = (int)strlen(plugin_dir);
		while(returned!=-1)
		{
			::ZeroMemory(DllFileName, 256);
			strncpy(DllFileName, plugin_dir, path_length - strlen("*.dll") );
			strcat(DllFileName, finddata.name);


			//load each Dll module and check for its validity	
			HINSTANCE hinst = LoadLibrary(DllFileName);
			if(hinst != NULL)
			{
				bool valid_dll = CheckForValidDll(hinst);
				if(valid_dll)
				{
					valid_dll = false;
					for(unsigned int i=0; i<app_ids.size();i++)
					{
						AppID id = ReturnAppID(hinst);
						if(id.IsEqual(app_ids[i])==1)
						{
							valid_dll = true;
							break;
						}
					}
					if(valid_dll)
					{
						DllInfo dll_info = GetInfoFromDll(hinst);
						if(IsDuplicateDll(dll_info)==false) // check to see if the same app_id is loaded already
						{
							valid_dll = true;
						}
						else
						{
							valid_dll = false;
						}
					}
					if(valid_dll)
					{
						typedef bool (*INIT)(string);
						INIT Init;
						Init = (INIT) GetProcAddress(hinst, "InitDll");
						if(Init != NULL)
						{
							valid_dll =  (Init)(dllloader_path); //Call InitDll
						}

						if(!valid_dll) //Call UnInitialize() for this Dll before freeing it's library
						{
							typedef void (*FREE)();
							FREE FreeDll;
							FreeDll = (FREE) GetProcAddress(hinst, "UnInitialize");
							(FreeDll)();
							FreeLibrary(hinst);
						}
						else
						{
							DllInfo dll_info = GetInfoFromDll(hinst);
							v_dlls.push_back(dll_info);
							m_num_dlls++;
						}
					}
				}
				if(!valid_dll)
					FreeLibrary(hinst);
			}
			returned=_findnext(hFile,&finddata);
		}
	}
	_findclose(hFile);
}

//
//
//
void DllManager::ReloadDlls()//vector<AppID> apps)
{
	//unload all dlls first
	typedef void (*FREE)();
	FREE FreeDll;
	vector<DllInfo>::iterator app_iter=v_dlls.begin();

	//Sleep(1000);	// To keep the Trapper Keeper alive

	while(app_iter!=v_dlls.end())
	{
		FreeDll = (FREE) GetProcAddress(app_iter->m_hinstance, "UnInitialize");
		(FreeDll)();
		FreeLibrary(app_iter->m_hinstance);
#ifdef DLL_DEBUG
		/* Temp debug message */
		char msg[256];
		sprintf(msg, "Unloaded the dll \"%s\", AppID: %d",app_iter->m_appID.m_app_name.c_str(), app_iter->m_appID.m_app_id);
		MessageBox(NULL,msg,"Trapper Keeper",MB_OK|MB_ICONINFORMATION);
#endif
		app_iter++;
	}

	v_dlls.clear();
	m_num_dlls = 0;

	//load the dlls in the vector
	if(v_reload_apps.size()>0)
		LoadDlls(v_reload_apps);

	//Update the new dlls list  in the dialog
	p_parent->UpdateDllList();

	StartAllDlls();
}

//
//
//
void DllManager::StartAllDlls(void)
{
	typedef void (*MYPROC)();
	MYPROC Function;

	vector<DllInfo>::iterator app_iter=v_dlls.begin();
	while(app_iter!=v_dlls.end())
	{
		Function = (MYPROC) GetProcAddress(app_iter->m_hinstance, "Start");
		(Function)();
		//ShowGUI(app_iter->m_appID);
		app_iter++;
	}
}

//
//
//
bool DllManager::IsDuplicateDll(DllInfo& info)
{
	bool found = false;
	vector<DllInfo>::iterator iter = v_dlls.begin();
	while(iter != v_dlls.end())
	{
		int ret = iter->m_appID.IsEqual(info.m_appID);
		if( ret == 1)	//The same dll is already loaded
		{
			found = true;
			UnInitializeDllOnly(info);
			break;
		}
		else if(ret == -1) //They are the same dll but different version
		{
			if(iter->m_appID.m_version > info.m_appID.m_version)	//the loaded dll is newer version, so unload this one
			{
				found = true;
				UnInitializeDllOnly(info);
				break;
			}
			else	//this one is newer version than the loaded one, so unload the old one
			{
				found = false;
				UnInitializeDllOnly(*(iter));
				v_dlls.erase(iter);
				break;
			}
		}
		iter++;
	}
	return found;
}

//
//
//
void DllManager::InitParent(DllLoaderApp* parent)
{
	p_parent = parent;
}
