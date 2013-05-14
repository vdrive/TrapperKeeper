#pragma once
#include "dll.h"
#include "../DllLoader/AppID.h"
//#include "SamplePlugInDlg.h"
#include "SupplySystem.h"

class ED2KSupplyProcessorDll :	public Dll
{
public:
	ED2KSupplyProcessorDll(void);
	~ED2KSupplyProcessorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
private:
	SupplySystemRef *mp_system;
//	CSamplePlugInDlg m_dlg;
};
