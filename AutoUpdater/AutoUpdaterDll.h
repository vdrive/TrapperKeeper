#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "AutoUpdaterDlg.h"
#include "AutoUpdaterSyncher.h"
#include "AutoUpdaterCom.h"

class AutoUpdaterDll :	public Dll
{
public:
	AutoUpdaterDll(void);
	~AutoUpdaterDll(void);
	bool ReceivedDllData(AppID from_app_id,void* input_data,void* output_data);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void PopulateCurrentDllListBox();
	void DataReceived(char *source_name, void *data, int data_length);

	void GetGoodDllsFromSource(char *source_ip);

private:
	CAutoUpdaterDlg m_dlg;
	AutoUpdaterSyncher m_AutoSyncher;
	AutoUpdaterCom m_AutoCom;

	vector<string> v_dll_strs;
};
