#pragma once
#include "..\sampleplugin\dll.h"
#include "KazaaSupplyDlg.h"

class KazaaSupplyTakerDll :
	public Dll
{
public:
	KazaaSupplyTakerDll(void);
	~KazaaSupplyTakerDll(void);


//	void DataReceived(char *source_ip, void *data, UINT data_length);
	bool SendData(AppID to_app_id, void* input_data = NULL,void* output_data = NULL);

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	bool ReceivedDllData(AppID from_app_id, void* input_data, void* output_data);

//private:
	CKazaaSupplyDlg m_dlg;
};
