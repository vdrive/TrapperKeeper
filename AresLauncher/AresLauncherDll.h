#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "AresLauncherDlg.h"

class AresLauncherDll :	public Dll
{
public:
	AresLauncherDll(void);
	~AresLauncherDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

	CAresLauncherDlg m_dlg;
	
};
