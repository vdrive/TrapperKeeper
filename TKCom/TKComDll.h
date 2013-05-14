#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "Resource.h"
#include "../SamplePlugIn/Interface.h"

//typical TrapperKeeper dll derived class
//Refer to ComService for project documentation.
class ComService;
class TKComDll : public Dll
{
public:
	TKComDll(void);
	~TKComDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	bool ReceivedDllData(AppID from_app_id,void* input_data,void* output_data);

private:
	//CDialog m_dlg;
	ComService *m_system;
};