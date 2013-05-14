#pragma once
#include "DllInfo.h"
#include "AppID.h"

class DllLoaderApp;
class DllManager
{
public:
	DllManager(void);
	~DllManager(void);
	void LoadAllDlls(void);
	void InitParent(DllLoaderApp* parent);
	static void ReloadDlls();//vector<AppID> apps);

	static vector<DllInfo> v_dlls;

private:
	static int m_num_dlls;

private:
	static DllInfo GetInfoFromDll(HINSTANCE hinst);
	static bool CheckForValidDll(HINSTANCE hinst);
	static AppID ReturnAppID(HINSTANCE hinst);
	static DllLoaderApp* p_parent;

public:

	/***********************************************/
	//		DllManager's export functions here     //
	/***********************************************/
	static bool		 DllManagerSendData(AppID from_app_id, AppID to_app_id, void* input_data = NULL, void* output_data = NULL);
	static void		 DllManagerReloadDlls(vector<AppID> apps);

	/***********************************************/
	//          DLL's exported functions here      */
	/***********************************************/

	static void Start(AppID& app_id);
	static bool ReceivedData(AppID from_app_id, AppID to_app_id, void* input_data, void* output_data);
	static void ShowGUI(AppID& app_id);
	static void UnInitialize(AppID& app_id);

	void FreeAllDlls(void);
	void ShowGUI(vector<UINT>& app_ids);
	void ShowGUI(UINT& app_id);
	static void LoadDlls(vector<AppID>& app_ids);
	static bool IsDuplicateDll(DllInfo& info);
private:
	static void StartAllDlls(void);
	static void UnInitializeDllOnly(DllInfo& info);
	static vector<AppID> v_reload_apps;

};

