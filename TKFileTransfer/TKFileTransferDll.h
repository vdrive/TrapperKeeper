#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "Resource.h"
#include "ComLink.h"

//typical TrapperKeeper dll derived class
//Refer to FileTransferService for project documentation.
class TKFileTransferDll :	public Dll
{
public:
	TKFileTransferDll(void);
	~TKFileTransferDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

private:

	bool ReceivedDllData(AppID from_app_id,void* input_data,void* output_data);
};
