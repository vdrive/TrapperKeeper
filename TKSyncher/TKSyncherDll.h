#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "Resource.h"
#include "SyncherDialog.h"
#include "SyncherService.h"
#include "SyncherComLink.h"
#include "ExecutableSource.h"
//#include "SamplePlugInDlg.h"

//Typical TrapperKeeper dll derived class.
//Refer to SyncherService.h for project documentation.
class TKSyncherDll :	public Dll
{
public:
	TKSyncherDll(void);
	~TKSyncherDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	bool ReceivedDllData(AppID from_app_id,void* input_data,void* output_data);

private:
	SyncherComLink m_com_link;
	//CSyncherDialog m_dialog;
	SyncherService m_service;
	ExecutableSource m_exec_source;
};
