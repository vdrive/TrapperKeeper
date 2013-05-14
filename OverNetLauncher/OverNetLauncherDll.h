#pragma once
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "OverNetLauncherDlg.h"

class OverNetLauncherDll :	public Dll
{
public:
	OverNetLauncherDll(void);
	~OverNetLauncherDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	COverNetLauncherDlg m_dlg;
};
