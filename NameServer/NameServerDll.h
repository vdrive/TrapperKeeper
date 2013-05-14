#pragma once
#include "..\sampleplugin\dll.h"
#include "NameServerDlg.h"
#include "xmlparser.h"
#include "RackInfo.h"
#define LENGTH 10000

//dll for the nameserver
class NameServerDll :public Dll
{
public:
	NameServerDll(void);
	~NameServerDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();	//shows gui
	bool ReceivedDllData(AppID from_app_id, void* input_data, void* output_data);

private:
	CNameServerDlg m_dlg;	//dialog object
	XMlParser parser;		//xml parser obejct
};
