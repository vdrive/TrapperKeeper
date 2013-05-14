#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
#include "WinMxProtectorDlg.h"

class WinMxProtectorDll :	public Dll
{
public:
	WinMxProtectorDll(void);
	~WinMxProtectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	// m_dlg;
};
